#include <alt_printf.h>
#include <alt_watchdog.h>
#include <hwlib.h>
#include <alt_generalpurpose_io.h>

#define LED_GPIO 53
#define LED_MASK (1 << 24)

int main( void )
{
  ALT_STATUS_CODE ret;
  ret = alt_wdog_uninit();

  alt_gpio_init();

  alt_gpio_port_datadir_set(ALT_GPIO_PORTB, LED_MASK, LED_MASK);

  printf("Hello World!!\n");
  while(1){
    alt_gpio_port_data_write(ALT_GPIO_PORTB, LED_MASK, LED_MASK);

    for(volatile int i=0; i<1000000; i++);

    alt_gpio_port_data_write(ALT_GPIO_PORTB, LED_MASK, 0);

    for(volatile int i=0; i<1000000; i++);
  };
  return 0;
}

