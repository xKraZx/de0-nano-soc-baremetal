extern "C" {
#include <alt_printf.h>
#include <alt_watchdog.h>
}

#include "fpga_image.hpp"
#include "fpga_loader.hpp"
#include "uart.hpp"
#include "benchmark.hpp"
#include "benchmarks.hpp"

int main()
{
    printf("INFO: ENTERED MAIN\n");
    alt_wdog_uninit();

    const FpgaImage &image = fpga_image_get();

    printf("INFO: FPGA image at %p, size = %u bytes.\n\n",
            image.data, static_cast<unsigned int>(image.size));

    if (fpga_load(image.data, image.size) != ALT_E_SUCCESS) {
        printf("ERROR: FPGA loading failed.\n");
    }

    printf("INFO: Initialization complete.\n\n");

    //Application app;
    //app.init();
    printf("Hell");

    while (true)
    {
        //app.process();
    }

    return 0;
}
