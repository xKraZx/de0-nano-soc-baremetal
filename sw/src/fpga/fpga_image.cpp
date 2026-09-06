
#include "fpga_image.hpp"

extern char rbf_start;
extern char rbf_end;

namespace
{
const FpgaImage image = {
    &rbf_start,
    static_cast<size_t>(&rbf_end - &rbf_start)
};
}

const FpgaImage &fpga_image_get()
{
    return image;
}
