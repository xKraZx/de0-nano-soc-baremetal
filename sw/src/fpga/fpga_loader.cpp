
extern "C" {
#include <alt_printf.h>
#include <alt_fpga_manager.h>
}

#include "fpga_loader.hpp"

ALT_STATUS_CODE fpga_load(const void *fpga_image, size_t fpga_image_size)
{
    ALT_STATUS_CODE status;

    printf("INFO: Initializing FPGA manager...\n");

    status = alt_fpga_init();
    if (status != ALT_E_SUCCESS) {
        printf(
            "ERROR: alt_fpga_init() failed: %" PRIi32 "\n",
            status);

        return status;
    }

    if (alt_fpga_state_get() == ALT_FPGA_STATE_POWER_OFF) {
        printf("ERROR: FPGA is powered off.\n");

        alt_fpga_uninit();
        return ALT_E_ERROR;
    }

    status = alt_fpga_control_enable();
    if (status != ALT_E_SUCCESS) {
        printf(
            "ERROR: alt_fpga_control_enable() failed: %" PRIi32 "\n",
            status);

        alt_fpga_uninit();
        return status;
    }

    printf("INFO: Loading FPGA image...\n");

    constexpr uint32_t max_retries = 5;

    for (uint32_t attempt = 1; attempt <= max_retries; ++attempt) {
        status = alt_fpga_configure(
            fpga_image,
            fpga_image_size);

        if (status == ALT_E_SUCCESS) {
            break;
        }

        printf(
            "WARN: FPGA configuration attempt %u failed: %" PRIi32 "\n",
            static_cast<unsigned int>(attempt),
            status);
    }

    if (status == ALT_E_SUCCESS) {
        printf("INFO: FPGA successfully configured.\n");
    } else {
        printf(
            "ERROR: FPGA configuration failed: %" PRIi32 "\n",
            status);
    }

    alt_fpga_control_disable();
    alt_fpga_uninit();

    return status;
}
