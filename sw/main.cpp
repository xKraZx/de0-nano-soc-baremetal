extern "C" {
#include <alt_printf.h>
#include <alt_watchdog.h>
}

#include "fpga_image.hpp"
#include "fpga_loader.hpp"
#include "h2f_bridge_demo.hpp"


int main( void )
{
    alt_wdog_uninit();

    const FpgaImage &image = fpga_image_get();

    printf("INFO: FPGA image at %p, size = %u bytes.\n",
            image.data, static_cast<unsigned int>(image.size));

    if (fpga_load(image.data, image.size) != ALT_E_SUCCESS) {
        printf("ERROR: FPGA loading failed.\n");
        goto error;
    }

    printf("INFO: Initialization complete.\n\n");

    if (h2f_bridge_demo_init() != ALT_E_SUCCESS) {
        printf("ERROR: H2F bridge demo initialization failed.\n");
        goto error;
    }

    if (h2f_bridge_demo_register_access() != ALT_E_SUCCESS) {
        printf("ERROR: H2F bridge register demo failed.\n");
        goto error;
    }

error:

    while(1);

    return 0;
}
