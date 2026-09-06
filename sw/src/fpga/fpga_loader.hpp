#pragma once

extern "C" {
#include <hwlib.h>
}

ALT_STATUS_CODE fpga_load(const void *fpga_image, size_t fpga_image_size);
