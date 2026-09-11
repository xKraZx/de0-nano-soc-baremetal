BUILD_DIR := build
COMPONENTS := components
HWLIB_DIR := $(COMPONENTS)/hwlib
UBOOT_DIR := $(BUILD_DIR)/uboot-socfpga
SRC_DIR := src
OBJ_DIR := $(BUILD_DIR)/obj
HW_PRJ_NAME := de0_nano_soc

UBOOT_ARCHIVE := $(COMPONENTS)/uboot_socfpga.tar.gz

TARGET 	:= $(HW_PRJ_NAME)_baremetal

CROSS_COMPILE := arm-none-eabi-
ARCH := arm

BSP_GENERATOR := $(UBOOT_DIR)/arch/arm/mach-socfpga/cv_bsp_generator/cv_bsp_generator.py
MKIMAGE := $(UBOOT_DIR)/tools/mkimage

CXX					:= $(CROSS_COMPILE)g++
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
	-I$(HWLIB_DIR)/include/safeclib

SRC_INCLUDES := $(shell find $(SRC_DIR) -type d)
SRC_INCLUDES := $(addprefix -I,$(SRC_INCLUDES))

DEFINES	:= -Dsoc_cv_av \
			-DALT_FPGA_ENABLE_DMA_SUPPORT=1 \
			-DALT_BRIDGE_PROVISION_F2S_SUPPORT \
			-DPRINTF_UART

LINKER_SCRIPT 		:= $(COMPONENTS)/cycloneV-dk-ram.ld
MULTILIBFLAGS 		:= -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a
LDFLAGS				:= -Wl,--gc-sections,-Map=$(BUILD_DIR)/$(TARGET).map
ASFLAGS 			:=  -march=armv7-a -mcpu=cortex-a9
CFLAGS 				:= $(MULTILIBFLAGS) -g -O0 -Wall \
							-fdata-sections -ffunction-sections \
							$(INCLUDES) \
							$(DEFINES)
CXXFLAGS			:= $(INCLUDES) \
						$(SRC_INCLUDES) \
						$(MULTILIBFLAGS) \
						$(DEFINES) \
						-g -O0 -Wall -Wextra -std=c++17 \
						-fdata-sections -ffunction-sections \
						-MMD -MP

LIBS := $(COMPONENTS)/libs

HWLIB_SRC := $(wildcard $(HWLIB_DIR)/src/safeclib/*.c) \
			$(HWLIB_DIR)/src/utils/alt_p2uart.c \
			$(HWLIB_DIR)/src/utils/alt_printf.c \
			$(wildcard $(HWLIB_DIR)/src/soc_cv_av/*.c) \
			$(wildcard $(HWLIB_DIR)/src/hwmgr/*.c) \
			$(wildcard $(HWLIB_DIR)/src/hwmgr/soc_cv_av/*.c) \
			$(HWLIB_DIR)/src/hwmgr/soc_cv_av/alt_bridge_f2s_gnu.s

SRC := $(HWLIB_SRC)\
		$(shell find $(SRC_DIR) -type f -name '*.cpp') \
		./main.cpp

OBJS := $(SRC)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.s=.o)
OBJS := $(addprefix $(OBJ_DIR)/,$(OBJS))

DEPS := $(OBJS:.o=.d)

-include $(DEPS)

$(HWLIB_SRC):
	$(shell cp -r $(HWLIB_ROOT) $(HWLIB_DIR) )
	$(shell mkdir -p $(OBJ_DIR)/$(HWLIB_DIR)/src/utils)
	$(shell mkdir -p $(OBJ_DIR)/$(HWLIB_DIR)/src/hwmgr/soc_cv_av)
	$(shell mkdir -p $(OBJ_DIR)/$(HWLIB_DIR)/src/safeclib)

#///////
IMG := sdcard.img
BAREMETAL_WITH_SPL := $(BUILD_DIR)/baremetal-with-spl.sfp
SPL := $(BUILD_DIR)/baremetal-spl.sfp
ELF := $(BUILD_DIR)/$(TARGET).axf
BIN := $(BUILD_DIR)/$(TARGET).bin
BINMKIMAGE 	:= $(BUILD_DIR)/$(TARGET)_mk.bin
RBF_OBJ := $(BUILD_DIR)/$(HW_PRJ_NAME).o
RBF := $(PROJECT_ROOT)/hw/$(HW_PRJ_NAME).rbf
CONFIG_STAMP := $(BUILD_DIR)/.config
QTS_STAMP := $(BUILD_DIR)/.qts
UNTAR_STAMP := $(BUILD_DIR)/.untar
#///////
.PHONY: all
all: $(BAREMETAL_WITH_SPL)

SPL_PAD := $(BUILD_DIR)/uboot-spl-pad.sfp
SPLX4   := $(BUILD_DIR)/uboot-splx4.sfp

$(BAREMETAL_WITH_SPL): $(SPL) $(BINMKIMAGE)
	$(OC) -I binary -O binary --gap-fill=0x0 \
		--pad-to=0x10000 \
		$(SPL) $(SPL_PAD)
	cat $(SPL_PAD) $(SPL_PAD) $(SPL_PAD) $(SPL_PAD) > $(SPLX4)
	cat $(SPLX4) $(BINMKIMAGE) > $@
#///////////
.PHONY: binary_mk
binary_mk: $(BINMKIMAGE)

$(BINMKIMAGE): $(BIN) $(ELF) $(MKIMAGE)
	ENTRYPOINT=$$($(RE) -h $(ELF) | awk '/Entry point address/ {print $$4}'); \
	$(MKIMAGE) -A arm -O u-boot  -T standalone -C none -a 0x00100000 -e $$ENTRYPOINT -n bare-metal-example  -d $(BIN) $(BINMKIMAGE)

.PHONY: binary_file
binary_file: $(BIN)

$(BIN): $(ELF)
	$(OC) -O binary $(ELF) $@

$(ELF): $(OBJS) $(RBF_OBJ)
	$(LD) -L$(LIBS) -T$(LINKER_SCRIPT) $(MULTILIBFLAGS) $(OBJS) $(LDFLAGS) $(RBF_OBJ) -o $@

$(OBJ_DIR)/%.o : %.cpp Makefile
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o : %.c Makefile
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s Makefile
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -c $< -o $@

$(RBF_OBJ): $(RBF)
	$(OC) -I binary -O elf32-littlearm -B arm \
		--redefine-sym _binary__project_hw_de0_nano_soc_rbf_start=rbf_start \
		--redefine-sym _binary__project_hw_de0_nano_soc_rbf_end=rbf_end \
		--redefine-sym _binary__project_hw_de0_nano_soc_rbf_size=rbf_size \
		$< $@

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
CONFIG_OVERRIDE := $(COMPONENTS)/$(TARGET).config

$(CONFIG_STAMP): $(UNTAR_STAMP)
	make -C $(UBOOT_DIR) $(UBOOT_CONFIG)
	cd $(UBOOT_DIR) && \
		./scripts/kconfig/merge_config.sh \
			.config \
			$(CURDIR)/$(CONFIG_OVERRIDE)
	make -C $(UBOOT_DIR) olddefconfig
	@touch $@
#///////////
.PHONY: generate-qts
generate-qts: $(QTS_STAMP)

$(QTS_STAMP): $(UNTAR_STAMP)
	python3 $(BSP_GENERATOR) -i $(PROJECT_ROOT)/hw/hps_isw_handoff/*/ -o $(UBOOT_DIR)/board/altera/cyclone5-socdk/qts
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
	rm -rf $(BUILD_DIR)