#include "osapi.h"
#include "user_interface.h"
#include <stdio.h>
#include "user_smart_config.h"

void ICACHE_FLASH_ATTR user_init()
{
<<<<<<< HEAD:user/user_main.c
	uart_div_modify(0,UART_CLK_FREQ/115200); 
    user_start_smartconfig();
=======
	uart_div_modify(0,UART_CLK_FREQ/115200);
  user_start_smartconfig();
>>>>>>> 2bd03566bd5849eeefbb02e1e4d36ebfa68a0b2b:user/user_main.c
}

