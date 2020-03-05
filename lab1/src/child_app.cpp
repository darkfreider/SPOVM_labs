
#include <stdio.h>
#include "app.h"

APPLICATION_MAIN(application_main)
{
	char time_buffer[256];
	for (;;)
	{
		App_time_info time = callbacks->platform_get_time();
		int time_buffer_len = sprintf(time_buffer, "Chiled: %hu-%hu-%hu\n", time.hour, time.minute, time.second);
		callbacks->platform_console_write(time_buffer, time_buffer_len);
		callbacks->platform_process_sleep(1);
	}
}
