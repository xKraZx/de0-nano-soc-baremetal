extern "C" {
#include <alt_printf.h>
#include <alt_fpga_manager.h>
#include <alt_bridge_manager.h>
#include <alt_address_space.h>
}

#include <stdint.h>

#include "h2f_bridge_demo.hpp"

namespace
{
constexpr uint32_t H2F_FPGA_BASE = 0xC0000000;
constexpr uint32_t DEMO_REG_OFFSET = 0x00010000;
constexpr uint32_t DEMO_TEST_VALUE = 0x45;
}

ALT_STATUS_CODE h2f_bridge_demo_init()
{
    ALT_STATUS_CODE status;

    printf("INFO: Initializing FPGA manager...\n");

    status = alt_fpga_init();
    if (status != ALT_E_SUCCESS) {
        printf(
            "ERROR: FPGA initialization failed. "
            "[status = %x]\n",
            status);

        return status;
    }

    printf("INFO: Initializing H2F bridge...\n");

    status = alt_bridge_init(
        ALT_BRIDGE_H2F,
        NULL,
        NULL);

    if (status != ALT_E_SUCCESS) {
        printf(
            "ERROR: H2F bridge initialization failed. "
            "[status = %x]\n",
            status);

        return status;
    }

    printf("INFO: Remapping H2F address space...\n");

    status = alt_addr_space_remap(
        ALT_ADDR_SPACE_MPU_ZERO_AT_BOOTROM,
        ALT_ADDR_SPACE_NONMPU_ZERO_AT_OCRAM,
        ALT_ADDR_SPACE_H2F_ACCESSIBLE,
        ALT_ADDR_SPACE_LWH2F_ACCESSIBLE);

    if (status != ALT_E_SUCCESS) {
        printf(
            "ERROR: H2F address space remap failed. "
            "[status = %x]\n",
            status);

        return status;
    }

    printf("INFO: H2F bridge initialized successfully.\n\n");

    return ALT_E_SUCCESS;
}

ALT_STATUS_CODE h2f_bridge_demo_register_access()
{
    const uint32_t address =
        H2F_FPGA_BASE + DEMO_REG_OFFSET;

    printf("INFO: H2F register access demo\n");

    printf(
        "INFO: Writing 0x%x to FPGA register at 0x%x\n",
        DEMO_TEST_VALUE,
        address);

    alt_write_word(address, DEMO_TEST_VALUE);

    const uint32_t value = alt_read_word(address);

    printf(
        "INFO: Read value 0x%x from FPGA register\n",
        value);

    if (value != DEMO_TEST_VALUE) {
        printf(
            "ERROR: H2F register readback mismatch. "
            "Expected 0x%x, got 0x%x\n",
            DEMO_TEST_VALUE,
            value);

        return ALT_E_ERROR;
    }

    printf("INFO: H2F register access successful.\n\n");

    return ALT_E_SUCCESS;
}
