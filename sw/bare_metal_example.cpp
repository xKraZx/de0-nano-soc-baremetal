extern "C" {
#include <alt_printf.h>
#include <alt_watchdog.h>
#include <hwlib.h>
#include <alt_address_space.h>
#include <alt_bridge_manager.h>
#include <alt_fpga_manager.h>
#include <socal.h>
#include <alt_dma.h>
}

static ALT_STATUS_CODE fpga_load(const void *fpga_image, size_t fpga_image_size)
{
    ALT_STATUS_CODE status;

    printf("INFO: Initializing FPGA manager...\n");

    status = alt_fpga_init();
    if (status != ALT_E_SUCCESS){
        printf("ERROR: alt_fpga_init() failed: %" PRIi32 "\n", status);
        return status;
    }

    if (alt_fpga_state_get() == ALT_FPGA_STATE_POWER_OFF){
        printf("ERROR: FPGA is powered off.\n");
        alt_fpga_uninit();
        return ALT_E_ERROR;
    }

    status = alt_fpga_control_enable();
    if (status != ALT_E_SUCCESS){
        printf("ERROR: alt_fpga_control_enable() failed: %" PRIi32 "\n", status);

        alt_fpga_uninit();
        return status;
    }

    ALT_FPGA_CFG_MODE_t mode = alt_fpga_cfg_mode_get();

    printf("FPGA MSEL mode = %x: ", mode);

    printf("FPGA state = 0x%x\n", alt_fpga_state_get());
    printf("INFO: Loading FPGA image...\n");

    const uint32_t retries = 5;

    for (uint32_t i = 0; i < retries; ++i){
        status = alt_fpga_configure(
            fpga_image,
            fpga_image_size);

        if (status == ALT_E_SUCCESS)
            break;

        printf(
            "WARN: FPGA configuration attempt %u failed: %" PRIi32 "\n",
            (unsigned int)(i + 1),
            status);
    }

    if (status == ALT_E_SUCCESS)
        printf("INFO: FPGA successfully configured.\n");
    else
        printf("ERROR: FPGA configuration failed: %" PRIi32 "\n", status);

    alt_fpga_control_disable();
    alt_fpga_uninit();

    return status;
}

ALT_STATUS_CODE socfpga_bridge_setup(ALT_BRIDGE_t bridge)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    printf("INFO: Initializing FPGA ...\n");

    if (status == ALT_E_SUCCESS){
        status = alt_fpga_init();
    } else {
        printf("ERROR: Initialization failed.\n");
        return status;
    }

    printf("INFO: Setup Bridge [%d] ...\n", bridge);

    if (status == ALT_E_SUCCESS){
        status = alt_bridge_init(bridge, NULL, NULL);
    }

    if (status == ALT_E_SUCCESS){
        status = alt_addr_space_remap(ALT_ADDR_SPACE_MPU_ZERO_AT_BOOTROM,
                                      ALT_ADDR_SPACE_NONMPU_ZERO_AT_OCRAM,
                                      ALT_ADDR_SPACE_H2F_ACCESSIBLE,
                                      ALT_ADDR_SPACE_LWH2F_ACCESSIBLE);
    }

    if (status == ALT_E_SUCCESS){
        printf("INFO: Setup of Bridge [%d] successful.\n\n", bridge);
    } else {
        printf("ERROR: Setup of Bridge [%d] failed. [status = %x ].\n\n", bridge, status);
    }

    return status;
}

ALT_STATUS_CODE socfpga_bridge_reg(void)
{
    const uint32_t ALT_H2F_FPGA_BASE       = 0xC0000000;
    const uint32_t ALT_H2F_REG_OFFSET      = 0x00010000;
    const uint32_t test_val                = 0x45;

    printf("Writing value: 0x%x , to FPGA register\n", test_val);

    alt_write_word(ALT_H2F_FPGA_BASE + ALT_H2F_REG_OFFSET, test_val);

    uint32_t reg_val = alt_read_word(ALT_H2F_FPGA_BASE + ALT_H2F_REG_OFFSET);

    printf("Reading from FPGA register: 0x%x\n", reg_val);

    return ALT_E_SUCCESS;
}

int main( void )
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;
    ALT_FPGA_STATE_t status_fpga;
    ALT_DMA_CHANNEL_t dma_channel;
    alt_wdog_uninit();

    if (status == ALT_E_SUCCESS){
        extern char rbf_start;
        extern char rbf_end;

        const char *   fpga_image      = &rbf_start;
        const uint32_t fpga_image_size = &rbf_end - &rbf_start;

        printf("INFO: FPGA Image binary at %p.\n", fpga_image);
        printf("INFO: FPGA Image size is %" PRIu32 " bytes.\n", fpga_image_size);

        status = fpga_load(fpga_image, fpga_image_size);
    }

    if (status == ALT_E_SUCCESS)
        printf("INFO: FPGA loading completed.\n");
    else
        printf("ERROR: FPGA loading failed.\n");

    if (status == ALT_E_SUCCESS){
        status = socfpga_bridge_setup(ALT_BRIDGE_H2F);
    }

    if (status == ALT_E_SUCCESS){
        status = socfpga_bridge_reg();
    }

    printf("Hello World!!\n");
    while(1);

    return 0;
}

