WORK_DIR := ../
TEMP_DIR := temp
COMPONENTS := components
HWLIB_DIR := $(COMPONENTS)/hwlib
UBOOT_DIR := $(TEMP_DIR)/uboot-socfpga
OBJDIR := $(TEMP_DIR)/obj

UBOOT_ARCHIVE := $(COMPONENTS)/uboot_socfpga.tar.gz

TARGET 	:= baremetal_app

CROSS_COMPILE := arm-none-eabi-
ARCH := arm

BSP_GENERATOR := $(UBOOT_DIR)/arch/arm/mach-socfpga/cv_bsp_generator/cv_bsp_generator.py
MKIMAGE := $(UBOOT_DIR)/tools/mkimage

CC 					:= $(CROSS_COMPILE)gcc
LD 					:= $(CROSS_COMPILE)g++
NM 					:= $(CROSS_COMPILE)nm
OD 					:= $(CROSS_COMPILE)objdump
OC 					:= $(CROSS_COMPILE)objcopy
AS 					:= $(CROSS_COMPILE)as
RE 					:= $(CROSS_COMPILE)readelf

#//////////////
INCLUDES := \
    -I$(HWLIB_DIR)/include \
    -I$(HWLIB_DIR)/include/soc_cv_av \
    -I$(HWLIB_DIR)/include/soc_cv_av/socal \
	-I$(HWLIB_DIR)/include/safeclib \

LINKER_SCRIPT 		:= $(COMPONENTS)/cycloneV-dk-ram.ld
MULTILIBFLAGS 		:= -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a
ASFLAGS 			:=  -march=armv7-a -mcpu=cortex-a9
CFLAGS 				:= $(MULTILIBFLAGS) -g -O0 -Wall \
							-fdata-sections -ffunction-sections \
							$(INCLUDES) \
						  -Dsoc_cv_av \
						  -DPRINTF_UART

LIBS := $(COMPONENTS)/libs

HWLIB_SRC := $(wildcard $(HWLIB_DIR)/src/safeclib/*.c) \
			$(HWLIB_DIR)/src/utils/alt_p2uart.c \
			$(HWLIB_DIR)/src/utils/alt_printf.c \
			$(wildcard $(HWLIB_DIR)/src/soc_cv_av/*.c) \
			$(wildcard $(HWLIB_DIR)/src/hwmgr/*.c) \
			$(wildcard $(HWLIB_DIR)/src/hwmgr/soc_cv_av/*.c)

SRC := $(HWLIB_SRC)\
		./bare_metal_example.c

OBJS := $(patsubst %.c,%.o,$(SRC)) 
OBJS := $(patsubst %.s,%.o,$(OBJS)) 
OBJS := $(patsubst %.o,$(OBJDIR)/%.o,$(OBJS)) 

$(HWLIB_SRC):
	$(shell cp -r $(HWLIB_ROOT) $(HWLIB_DIR) )
	$(shell mkdir -p $(OBJDIR)/$(HWLIB_DIR)/src/utils)
	$(shell mkdir -p $(OBJDIR)/$(HWLIB_DIR)/src/hwmgr/soc_cv_av)
	$(shell mkdir -p $(OBJDIR)/$(HWLIB_DIR)/src/safeclib)

#///////
IMG := sdcard.img
BAREMETAL_WITH_SPL := $(TEMP_DIR)/baremetal-with-spl.sfp
SPL := $(TEMP_DIR)/baremetal-spl.sfp
ELF := $(TEMP_DIR)/$(TARGET).axf
BIN := $(TEMP_DIR)/$(TARGET).bin
BINMKIMAGE 	:= $(TEMP_DIR)/$(TARGET)_mk.bin
CONFIG_STAMP := $(TEMP_DIR)/.config
QTS_STAMP := $(TEMP_DIR)/.qts
UNTAR_STAMP := $(TEMP_DIR)/.untar
#///////
.PHONY: all
all: $(BAREMETAL_WITH_SPL)

SPL_PAD := $(TEMP_DIR)/uboot-spl-pad.sfp
SPLX4   := $(TEMP_DIR)/uboot-splx4.sfp

$(BAREMETAL_WITH_SPL): $(SPL) $(BINMKIMAGE)
	$(OC) -I binary -O binary --gap-fill=0x0 \
		--pad-to=0x10000 \
		$(SPL) $(SPL_PAD)
	cat $(SPL_PAD) $(SPL_PAD) $(SPL_PAD) $(SPL_PAD) > $(SPLX4)
	cat $(SPLX4) $(BINMKIMAGE) > $@
#///////////
.PHONY: binary_mk
binary_mk: $(BINMKIMAGE)

$(BINMKIMAGE): $(BIN)
	$(MKIMAGE) -A arm -O u-boot  -T standalone -C none -a 0x00100000 -e 0x00100000 -n bare-metal-example  -d $(BIN) $(BINMKIMAGE)

#$(BINMKIMAGE): $(BIN) $(ELF) $(MKIMAGE)
#	ENTRYPOINT=$$($(RE) -h $(ELF) | awk '/Entry point address/ {print $$4}'); \
#	$(MKIMAGE) -A arm -O u-boot  -T standalone -C none -a 0x00100000 -e $$ENTRYPOINT -n bare-metal-example  -d $(BIN) $(BINMKIMAGE)

.PHONY: binary_file
binary_file: $(BIN)

$(BIN): $(ELF)
	$(OC) -O binary $(ELF) $@

$(ELF): $(OBJS)
	$(LD) -L$(LIBS) -T$(LINKER_SCRIPT) $(MULTILIBFLAGS) $(OBJS) -o $@

$(OBJDIR)/%.o : %.c Makefile
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s Makefile
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -c $< -o $@

#///////////
.PHONY: spl
spl: $(SPL)

$(SPL): $(CONFIG_STAMP) $(QTS_STAMP)
	make CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) -j$$(nproc) -C $(UBOOT_DIR) spl/u-boot-spl.sfp
	cp $(UBOOT_DIR)/spl/u-boot-spl.sfp $@
#///////////
.PHONY: build-mkimage
build-mkimage: $(MKIMAGE)

$(MKIMAGE): $(UNTAR_STAMP) $(CONFIG_STAMP)
	make -C $(UBOOT_DIR) tools/mkimage
#///////////
.PHONY: config
config: $(CONFIG_STAMP)

UBOOT_CONFIG := socfpga_de0_nano_soc_defconfig

$(CONFIG_STAMP): $(UNTAR_STAMP)
	make -C $(UBOOT_DIR) $(UBOOT_CONFIG)
	@touch $@
#///////////
.PHONY: generate-qts
generate-qts: $(QTS_STAMP)

$(QTS_STAMP): $(UNTAR_STAMP)
	python3 $(BSP_GENERATOR) -i $(WORK_DIR)/hw/hps_isw_handoff/*/ -o $(UBOOT_DIR)/board/altera/cyclone5-socdk/qts
	@touch $@
#///////////
.PHONY: unpack-uboot
unpack-uboot: $(UNTAR_STAMP)

$(UNTAR_STAMP): $(UBOOT_ARCHIVE)
	@rm -rf $(UBOOT_DIR)
	@mkdir -p $(UBOOT_DIR)
	tar -xzf "$(UBOOT_ARCHIVE)" -C $(UBOOT_DIR)
	@touch $@
#///////////

.PHONY: clean
clean:
	rm -rf $(TEMP_DIR)