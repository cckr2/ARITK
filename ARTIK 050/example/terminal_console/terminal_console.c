#include "wiced.h"
#include <string.h>
#include "../terminal_console/js_console.h"

#define CONSOLE_COMMAND_HISTORY_LENGTH  (10)
#define MAX_COMMAND_LENGTH              (85)
#define WICED_DELAY_IN_MILLISECONDS     (50)

static char                     command_buffer[MAX_COMMAND_LENGTH];
static char                     command_history_buffer[MAX_COMMAND_LENGTH * CONSOLE_COMMAND_HISTORY_LENGTH];


void myfun(char *str){
	WPRINT_APP_INFO(("%s\n",str));
}

void application_start( void )
{
	  wiced_result_t result;
	  WPRINT_APP_INFO( ( "console start\n") );

      result = console_init(STDIO_UART, sizeof(command_buffer), command_buffer, CONSOLE_COMMAND_HISTORY_LENGTH, command_history_buffer, " ", myfun);

      if (result != WICED_SUCCESS)
      {
              WPRINT_APP_INFO(("ERROR: Starting the command console\r\n"));
      }
      console_start();
}
