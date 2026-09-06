#pragma once

#include <stddef.h>

struct FpgaImage
{
    const void *data;
    size_t size;
};

const FpgaImage &fpga_image_get();
