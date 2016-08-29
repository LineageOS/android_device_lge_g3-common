LOCAL_PATH := $(call my-dir)

## Build and run dtbtool
BUMP := $(LOCAL_PATH)/bump/bump.py
DTBTOOL := $(HOST_OUT_EXECUTABLES)/dtbToolCM$(HOST_EXECUTABLE_SUFFIX)
INSTALLED_DTIMAGE_TARGET := $(PRODUCT_OUT)/dt.img

$(INSTALLED_DTIMAGE_TARGET): $(DTBTOOL) $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr $(INSTALLED_KERNEL_TARGET)
	$(hide) $(DTBTOOL) -o $(INSTALLED_DTIMAGE_TARGET) -s $(BOARD_KERNEL_PAGESIZE) -p $(KERNEL_OUT)/scripts/dtc/ $(KERNEL_OUT)/arch/arm/boot/
	@echo "Made DT image: $@"


LZMA_BOOT_RAMDISK := $(PRODUCT_OUT)/ramdisk-lzma.img

$(LZMA_BOOT_RAMDISK): $(BUILT_RAMDISK_TARGET)
	gunzip -f < $(BUILT_RAMDISK_TARGET) | lzma -9 > $@

## Overload bootimg generation: Same as the original, + --dt arg
$(INSTALLED_BOOTIMAGE_TARGET): $(MKBOOTIMG) $(INTERNAL_BOOTIMAGE_FILES) $(INSTALLED_DTIMAGE_TARGET) $(LZMA_BOOT_RAMDISK)
	$(hide) $(MKBOOTIMG) $(INTERNAL_BOOTIMAGE_ARGS) $(BOARD_MKBOOTIMG_ARGS) --dt $(INSTALLED_DTIMAGE_TARGET) --output $@ --ramdisk $(LZMA_BOOT_RAMDISK)
ifeq ($(TARGET_REQUIRES_BUMP),true)
	$(hide) $(BUMP) $@ $@
endif
	$(hide) $(call assert-max-image-size,$@,$(BOARD_BOOTIMAGE_PARTITION_SIZE))
	@echo "Made boot image: $@"


LZMA_RECOVERY_RAMDISK := $(PRODUCT_OUT)/ramdisk-recovery-lzma.img

$(LZMA_RECOVERY_RAMDISK): $(recovery_ramdisk)
	gunzip -f < $(recovery_ramdisk) | lzma -9 > $@

## Overload recoveryimg generation: Same as the original, + --dt arg
$(INSTALLED_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) $(INSTALLED_DTIMAGE_TARGET) $(LZMA_RECOVERY_RAMDISK) $(recovery_kernel)
	$(hide) $(MKBOOTIMG) $(INTERNAL_RECOVERYIMAGE_ARGS) $(BOARD_MKBOOTIMG_ARGS) --dt $(INSTALLED_DTIMAGE_TARGET) --output $@ --ramdisk $(LZMA_RECOVERY_RAMDISK)
ifeq ($(TARGET_REQUIRES_BUMP),true)
	$(hide) $(BUMP) $@ $@
endif
	$(hide) $(call assert-max-image-size,$@,$(BOARD_RECOVERYIMAGE_PARTITION_SIZE))
	@echo "Made recovery image: $@"
