#!/usr/bin/env python3
#
# Creates an SD card image for Altera SoCFPGA SoCs
# Supports:
#   vfat / fat / fat32
#   ext2 / ext3 / ext4
#   xfs
#   raw / none -> MBR partition type A2
#

import os
import sys
import re
import glob
import argparse
import textwrap
import subprocess
import time

MAX_PARTITIONS = 4

# Globals
loopback_dev_used = []
mounted_fs = []


# ============================================================================
# Convert to bytes
def convert_size_from_unit(unit_size):

    factor = 1

    m = re.match(r"^[0-9]+[KMG]?$", unit_size, re.I)

    if m is None:
        print("error: " + unit_size + ": malformed expression")
        sys.exit(-1)

    munit = re.search(r"[KMG]+$", m.group(0), re.I)
    msize = re.search(r"^[0-9]+", m.group(0), re.I)

    if munit:
        unit = munit.group(0).upper()

        if unit == 'K':
            factor = 1024
        elif unit == 'M':
            factor = 1024 * 1024
        elif unit == 'G':
            factor = 1024 * 1024 * 1024

    size = int(convert_str_to_int(msize.group(0)) * factor)

    return int(size)


# ============================================================================
# Converts a string to int
def convert_str_to_int(string):

    try:
        integer = int(string)
    except ValueError:
        print("error: " + string + ": not a valid number")
        sys.exit(-1)

    return integer


# ============================================================================
# Checks requested filesystem format
def validate_format(fs_format):

    match = re.search(
        r"^(ext[2-4]|xfs|fat32|vfat|fat|none|raw)$",
        fs_format,
        re.I
    )

    return match is not None


# ============================================================================
# Parse one -P argument
def parse_single_part_args(part):

    part_entries = {}
    part_entries['files'] = []

    p = re.compile(r"[a-zA-Z0-9_]+=")

    for el in part.split(","):

        if p.match(el):

            key, value = el.split("=", 1)

            if value == "":
                print("error:", key, ": no value found.")
                sys.exit(-1)

            if key == 'num':
                part_entries[key] = convert_str_to_int(value)

            elif key == 'size':
                size = convert_size_from_unit(value)
                part_entries[key] = int(size)

            elif key == 'format':

                if validate_format(value):
                    part_entries[key] = value.lower()
                else:
                    print("error:", value, "unknown format")
                    sys.exit(-1)

            elif key == 'type':
                part_entries[key] = value

            else:
                print("error:", key, ": unknown option")
                sys.exit(-1)

        else:
            part_entries['files'].append(el)

    return part_entries


# ============================================================================
# Parse all -P arguments
def parse_all_parts_args(part_args):

    part_entries = {}

    if part_args is None:
        print("error: at least one -P partition must be specified")
        sys.exit(-1)

    num_args = len(part_args)

    if num_args > MAX_PARTITIONS:
        print("error: up to " + str(MAX_PARTITIONS) + " partitions allowed")
        sys.exit(-1)

    for part in part_args:

        part_entry = parse_single_part_args(part)

        if 'num' not in part_entry:
            print("error: partition number must be specified")
            sys.exit(-1)

        if part_entry['num'] in part_entries.keys():
            print(
                "error:",
                str(part_entry['num']),
                ": partition already used"
            )
            sys.exit(-1)

        part_entries[part_entry['num']] = part_entry

    return part_entries


# ============================================================================
# Derive fdisk partition type from filesystem format
def derive_fdisk_type_from_format(pformat):

    if re.match(r'^ext[2-4]$|^xfs$', pformat, re.I):
        return '83'

    elif re.match(r'^vfat$|^fat$|^fat32$', pformat, re.I):
        return 'b'

    # RAW partition -> A2
    elif re.match(r'^raw$|^none$', pformat, re.I):
        return 'A2'

    else:
        print("error:", pformat, ": unknown format")
        sys.exit(-1)


# ============================================================================
# Derive fdisk partition type from user supplied type
def derive_fdisk_type_from_ptype(ptype):

    if re.match(r'^(raw|none)$', ptype, re.I):
        return 'A2'

    elif ptype.lower() == 'swap':
        return '84'

    # Allow direct hexadecimal partition IDs, e.g. type=83, type=A2
    elif re.match(r'^[0-9a-fA-F]{1,2}$', ptype):
        return ptype

    else:
        print("error:", ptype, ": unknown type")
        sys.exit(-1)


# ============================================================================
# Check partition definitions and calculate offsets
def check_and_update_part_entries(part_entries, image_size):

    offset = 2048       # sectors, 1 MiB
    total_size = 0

    for part in sorted(part_entries.keys()):

        entry = part_entries[part]

        # Size required
        if 'size' not in entry:
            print("error:", part, ": size must be specified")
            sys.exit(-1)

        if entry['size'] == 0:
            print("error:", part, ": size is 0")
            sys.exit(-1)

        total_size += entry['size']

        # Determine partition type
        if 'format' not in entry:

            if 'type' not in entry:
                print(
                    "error:",
                    part,
                    ": specify at least format or type"
                )
                sys.exit(-1)

            entry['fdisk_type'] = derive_fdisk_type_from_ptype(
                entry['type']
            )

            # If type=raw/none, internally treat it as RAW
            if entry['type'].lower() in ('raw', 'none'):
                entry['format'] = 'raw'

        else:

            if 'type' not in entry:
                entry['fdisk_type'] = derive_fdisk_type_from_format(
                    entry['format']
                )
            else:
                entry['fdisk_type'] = derive_fdisk_type_from_ptype(
                    entry['type']
                )

        # Partition start sector
        entry['start'] = int(offset)

        # Size in sectors
        bsize = (
            entry['size'] // 512 +
            (1 if entry['size'] % 512 else 0)
        )

        entry['bsize'] = int(bsize)

        # Next partition starts one sector after previous one
        offset += int(bsize) + 1

    if total_size > image_size:
        print("error: partitions are too big to fit in image")
        sys.exit(-1)

    return part_entries


# ============================================================================
# Root check
def is_user_root():
    return os.getuid() == 0


# ============================================================================
# Check if file exists
def check_file_exists(filename):
    return os.path.isfile(filename)


# ============================================================================
# Create empty image
def create_empty_image(image_name, image_size, force_erase_image):

    if check_file_exists(image_name):

        if force_erase_image is False:

            try:
                answer = input(
                    "the image " +
                    image_name +
                    " exists. Remove? [y|n] "
                )
            except EOFError:
                answer = 'n'

        else:
            answer = 'Y'

        if answer.lower() == 'y':

            try:
                os.remove(image_name)
            except OSError:
                print(
                    "error: failed to remove " +
                    image_name +
                    ". Exit"
                )
                sys.exit(-1)

            print("image removed")

        else:
            print("user declined")
            return False

    try:

        subprocess.check_output(
            [
                "dd",
                "if=/dev/zero",
                "of=" + image_name,
                "bs=1",
                "count=0",
                "seek=" + str(image_size)
            ],
            stderr=subprocess.STDOUT
        )

    except subprocess.CalledProcessError:

        print("error: failed to create the image")
        sys.exit(-1)

    return True


# ============================================================================
# Create loopback device
def create_loopback(image_name, size, offset=0):

    try:

        if offset != 0:

            device = subprocess.check_output(
                [
                    "losetup",
                    "--show",
                    "-f",
                    "-o",
                    str(offset),
                    "--sizelimit",
                    str(size),
                    image_name
                ]
            )

        else:

            device = subprocess.check_output(
                [
                    "losetup",
                    "--show",
                    "-f",
                    "--sizelimit",
                    str(size),
                    image_name
                ]
            )

    except subprocess.CalledProcessError:

        print("error: failed to get a loopback device")
        clean_up()
        sys.exit(-1)

    device = device.decode('utf-8').rstrip()

    loopback_dev_used.append(device)

    return device


# ============================================================================
# Delete loopback
def delete_loopback(device):

    try:

        subprocess.check_output(
            ["losetup", "-d", str(device)],
            stderr=subprocess.STDOUT
        )

    except subprocess.CalledProcessError:

        return False

    if device in loopback_dev_used:
        loopback_dev_used.remove(device)

    return True


# ============================================================================
# Cleanup
def clean_up():

    for mp in mounted_fs[:]:
        umount_fs(mp)

    for device in loopback_dev_used[:]:

        if not delete_loopback(device):
            print(
                "error: could not delete loopback device",
                device
            )

    return 0


#==============================================================================
# Create MBR partition table using sfdisk
def create_partition_table(loopback, partition_entries):

    print("info: creating partition table using sfdisk")

    # sfdisk works with sectors.
    # We explicitly create a DOS/MBR partition table and specify
    # the partition type as hexadecimal 0xA2.

    lines = []
    lines.append("label: dos")
    lines.append("unit: sectors")

    for part in sorted(partition_entries.keys()):

        pentry = partition_entries[part]

        start = int(pentry['start'])
        size = int(pentry['bsize'])
        ptype = str(pentry['fdisk_type']).lower()

        # For raw/none force MBR type A2
        if re.match(r'^(raw|none)$', pentry.get('format', ''), re.I):
            ptype = "0xa2"

        lines.append(
            "/dev/null : start=%d, size=%d, type=%s"
            % (start, size, ptype)
        )

    script = "\n".join(lines) + "\n"

    print("info: sfdisk commands:")
    print(script)

    try:
        p = subprocess.Popen(
            ["sfdisk", "--no-reread", loopback],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True
        )

        out, err = p.communicate(script)

    except OSError as e:
        print("error: sfdisk: system error:", e)
        clean_up()
        sys.exit(-1)

    if p.returncode != 0:
        print("error: sfdisk failed")
        print("stdout:")
        print(out)
        print("stderr:")
        print(err)
        clean_up()
        sys.exit(-1)

    print("info: partition table created")

    return


# ============================================================================
# Map format to mkfs command
def get_mkfs_from_format(pformat):

    if re.search(r"^ext[2-4]$", pformat, re.I):
        return "mkfs." + pformat

    elif re.search(r"fat|vfat|fat32", pformat, re.I):
        return "mkfs.vfat"

    elif re.search(r"^xfs$", pformat, re.I):
        return "mkfs.xfs"

    return ""


# ============================================================================
# Map format to mkfs parameters
def get_mkfs_params_from_format(pformat):

    params = []

    if re.search(r"fat32", pformat, re.I):
        params = ["-F", "32"]

    elif re.search(r"vfat", pformat, re.I):
        params = ["-I"]

    return params


# ============================================================================
# Format filesystem partition
#
# RAW partitions are NEVER passed here.
def format_partition(loopback, fs_format):

    cmd = get_mkfs_from_format(fs_format)
    params = get_mkfs_params_from_format(fs_format)

    if not cmd:
        return

    try:

        p = subprocess.Popen(
            [cmd, loopback] + params,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True
        )

        out, err = p.communicate()

    except OSError as e:

        print("error: format: failed to execute", cmd)
        print("error:", e)

        clean_up()
        sys.exit(-1)

    if p.returncode != 0:

        print(
            "error: format: failed. Return code=%d"
            % p.returncode
        )

        print("params=%s, cmd=%s" % (params, cmd))
        print("stdout=%s" % out)
        print("stderr=%s" % err)

        clean_up()
        sys.exit(-1)


# ============================================================================
# Get mount filesystem
def get_mountfs_from_format(pformat):

    if re.search(r"fat32|fat", pformat, re.I):
        return "vfat"

    return pformat


# ============================================================================
# Mount filesystem
def mount_fs(loopback, fs_format):

    mp = (
        "/tmp/" +
        str(int(time.time())) +
        "_" +
        str(os.getpid())
    )

    try:
        os.mkdir(mp)

    except OSError:

        print(
            "error: failed to create mount point (",
            mp,
            ")"
        )

        clean_up()
        sys.exit(-1)

    format_name = get_mountfs_from_format(fs_format)

    p = subprocess.Popen(
        ["mount", "-t", format_name, loopback, mp],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True
    )

    out, err = p.communicate()

    if p.returncode != 0:

        print(
            "error: mount: failed (",
            loopback,
            mp,
            ")"
        )

        print("stdout=%s" % out)
        print("stderr=%s" % err)

        clean_up()
        sys.exit(-1)

    mounted_fs.append(mp)

    return mp


# ============================================================================
# Unmount filesystem
def umount_fs(mp):

    time.sleep(1)

    p = subprocess.Popen(
        ["umount", mp],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True
    )

    out, err = p.communicate()

    if p.returncode != 0:

        print("error: failed to umount", mp)
        print("stdout=%s" % out)
        print("stderr=%s" % err)

        sys.exit(-1)

    if mp in mounted_fs:
        mounted_fs.remove(mp)

    try:
        os.rmdir(mp)
    except OSError:
        pass

    return


# ============================================================================
# Raw copy
def do_raw_copy(loopback, partition_data):

    offset = 0

    for stuff in partition_data['files']:

        if os.path.isdir(stuff):

            print(
                "error:",
                stuff,
                ": can't copy dirs to raw partitions"
            )

            clean_up()
            sys.exit(-1)

        print(
            "info: RAW copy",
            stuff,
            "->",
            loopback
        )

        try:

            p = subprocess.Popen(
                [
                    "dd",
                    "if=" + stuff,
                    "of=" + loopback,
                    "bs=1",
                    "seek=" + str(offset),
                    "status=progress"
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                universal_newlines=True
            )

            out, err = p.communicate()

        except OSError as e:

            print("error:", e)
            clean_up()
            sys.exit(-1)

        if p.returncode != 0:

            print(
                "error:",
                stuff,
                ": failed to do raw copy"
            )

            print("stdout=%s" % out)
            print("stderr=%s" % err)

            clean_up()
            sys.exit(-1)

        offset += os.stat(stuff).st_size

    return


# ============================================================================
# Copy files to filesystem
def do_copy(loopback, partition_data):

    mp = mount_fs(
        loopback,
        partition_data['format']
    )

    for stuff in partition_data['files']:

        if os.path.isdir(stuff):
            stuff = stuff + "/*"

        if re.search(
            r"^fat|vfat|fat32$",
            partition_data['format'],
            re.I
        ):
            cp_opt = "-rt"
        else:
            cp_opt = "-at"

        try:

            p = subprocess.Popen(
                ["cp", cp_opt, mp] + glob.glob(stuff),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                universal_newlines=True
            )

            out, err = p.communicate()

            if p.returncode:
                raise Exception()

        except Exception:

            print("error: failed to copy", stuff)
            print("stdout=%s" % out)
            print("stderr=%s" % err)

            clean_up()
            sys.exit(-1)

    umount_fs(mp)

    return


# ============================================================================
# Copy files according to partition format
def copy_files_to_partition(loopback, partition_data):

    pformat = partition_data.get('format', '').lower()

    if pformat in ('raw', 'none'):

        # RAW partition:
        # no filesystem
        # no mount
        # files are written directly using dd

        do_raw_copy(
            loopback,
            partition_data
        )

    else:

        do_copy(
            loopback,
            partition_data
        )

    return


# ============================================================================
# Create, format and copy files to partition
def do_partition(partition, image_name):

    print("info: processing partition", partition['num'])

    offset_bytes = int(
        partition['start'] * 512
    )

    pformat = partition.get('format', '').lower()

    print(
        "     start sector:",
        partition['start']
    )

    print(
        "     size:",
        partition['size'],
        "bytes"
    )

    print(
        "     format:",
        pformat
    )

    print(
        "     fdisk type:",
        partition['fdisk_type']
    )

    if (
        pformat == "fat32" and
        partition['size'] < 33554432
    ):

        print(
            "error: Unable to create a fat32 partition "
            "size < 32MB"
        )

        sys.exit(-1)

    loopback = create_loopback(
        image_name,
        partition['size'],
        offset_bytes
    )

    # ================================================================
    # IMPORTANT:
    # RAW/A2 partitions must NOT be formatted.
    # ================================================================

    if pformat not in ("raw", "none"):

        format_partition(
            loopback,
            pformat
        )

    else:

        print(
            "info: RAW/A2 partition - "
            "filesystem formatting skipped"
        )

    # Copy contents
    copy_files_to_partition(
        loopback,
        partition
    )

    time.sleep(1)

    if not delete_loopback(loopback):

        clean_up()
        sys.exit(-1)

    return


# ============================================================================
# Create complete image
def create_image(
    image_name,
    image_size,
    partition_entries,
    force_erase_image
):

    print(
        "info: creating the image",
        image_name
    )

    # Create empty image
    if not create_empty_image(
        image_name,
        image_size,
        force_erase_image
    ):

        print(
            "error: the image file could not be created"
        )

        sys.exit(-1)

    # Create partition table
    print("info: creating the partition table")

    loopback = create_loopback(
        image_name,
        image_size
    )

    create_partition_table(
        loopback,
        partition_entries
    )

    print("info: deleting loopback")

    if not delete_loopback(loopback):

        clean_up()
        sys.exit(-1)

    # Process partitions
    print("info: processing partitions...")

    for part in sorted(partition_entries.keys()):

        print(
            "     partition #" +
            str(part) +
            "..."
        )

        do_partition(
            partition_entries[part],
            image_name
        )

    return


# ============================================================================
# MAIN
# ============================================================================

parser = argparse.ArgumentParser(
    description=(
        "Creates an SD card image for "
        "Altera's SoCFPGA SoCs"
    ),

    epilog=textwrap.dedent(
        """\
        Usage:
          PROG [-h] -P <partition info> [-P ...]

        Example RAW/A2:
          PROG -s 8G -n sdcard.img -P image.bin,num=1,format=raw,size=64M
        """
    )
)

parser.add_argument(
    '-P',
    dest='part_args',
    action='append',
    required=True,
    help=(
        'specifies a partition. May be used multiple times. '
        'file[,file,...],num=<part_num>,'
        'format=<vfat|fat32|ext[2-4]|xfs|raw>,'
        'size=<num[K|M|G]>[,type=ID]'
    )
)

parser.add_argument(
    '-s',
    dest='size',
    action='store',
    default='8G',
    help=(
        'specifies the size of the image. '
        'Units K|M|G can be used.'
    )
)

parser.add_argument(
    '-n',
    dest='image_name',
    action='store',
    default='somename.img',
    help='specifies the name of the image.'
)

parser.add_argument(
    '-f',
    dest='force_erase_image',
    action='store_true',
    default=False,
    help='deletes the image file if exists'
)

args = parser.parse_args()


# ============================================================================
# Root check
if not is_user_root():

    print("error: only root can do this...")
    sys.exit(-1)


# ============================================================================
# Parse arguments
part_entries = parse_all_parts_args(
    args.part_args
)

image_size = int(
    convert_size_from_unit(args.size)
)

part_entries = check_and_update_part_entries(
    part_entries,
    image_size
)


# ============================================================================
# Create image
create_image(
    args.image_name,
    image_size,
    part_entries,
    args.force_erase_image
)

print(
    "info: image created, file name is",
    args.image_name
)
