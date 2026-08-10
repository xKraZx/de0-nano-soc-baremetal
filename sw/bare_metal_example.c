#include <alt_printf.h>
#include <alt_watchdog.h>
#include <hwlib.h>
#include <alt_address_space.h>
#include <alt_bridge_manager.h>
#include <alt_fpga_manager.h>
#include <socal.h>


ALT_STATUS_CODE socfpga_bridge_setup(ALT_BRIDGE_t bridge)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    printf("INFO: Setup Bridge [%d] ...\n", bridge);

    if (status == ALT_E_SUCCESS)
    {
        status = alt_bridge_init(bridge, NULL, NULL);
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_addr_space_remap(ALT_ADDR_SPACE_MPU_ZERO_AT_BOOTROM,
                                      ALT_ADDR_SPACE_NONMPU_ZERO_AT_OCRAM,
                                      ALT_ADDR_SPACE_H2F_ACCESSIBLE,
                                      ALT_ADDR_SPACE_LWH2F_ACCESSIBLE);
    }

    if (status == ALT_E_SUCCESS)
    {
        printf("INFO: Setup of Bridge [%d] successful.\n\n", bridge);
    }
    else
    {
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
    ALT_STATUS_CODE ret = ALT_E_SUCCESS;
    ALT_FPGA_STATE_t status_fpga;
    alt_wdog_uninit();


    ret = alt_fpga_init();

    if (ret != ALT_E_SUCCESS){
        printf("FPGA manager init failed\n");
    }

    while ((status_fpga = alt_fpga_state_get()) != ALT_FPGA_STATE_USER_MODE){
        printf("FPGA is not initialized: [status = %x ].\n\n", status_fpga);
        for(volatile int i=0; i<2000000; i++);
    }

    if (ret == ALT_E_SUCCESS){
        ret = socfpga_bridge_setup(ALT_BRIDGE_H2F);
    }

    if (ret == ALT_E_SUCCESS){
        ret = socfpga_bridge_reg();
    }

    printf("Hello World!!\n");
    while(1);

    return 0;
}

