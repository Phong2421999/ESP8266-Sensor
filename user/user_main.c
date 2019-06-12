#include "osapi.h"
#include "user_interface.h"
#include <stdio.h>
#include "user_smart_config.h"

void ICACHE_FLASH_ATTR user_init()
{
	uart_div_modify(0,UART_CLK_FREQ/115200);
  user_start_smartconfig();
}

