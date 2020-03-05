
#include "app.h"

#if defined(GENERATE_PARENT_APP)
	#include "parent_app.cpp"
#elif defined(GENERATE_CHILD_APP)
	#include "child_app.cpp"
#else
	#error "specify target to generate!"
#endif

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <ncurses.h>

PLATFORM_PROCESS_CREATE(platform_process_create)
{
	int result = 0xFF;

	pid_t fres = fork();
	if (fres == (-1))
	{
		result = 0;
	}
	else if (fres == 0)
	{
		char buffer[128];
		snprintf(buffer, sizeof(buffer), "/bin/bash -c '%s; exec /bin/bash -i'", module_name);
				
		// child
		char * const args[] = {
                        (char *const)"gnome-terminal",
                        (char *const)"-e",
                        (char *const)buffer,
			0
                };

                if (execv("/usr/bin/gnome-terminal", args) == -1)
                {
			result = 0;
                }
	}

	return (result);
}

PLATFORM_CONSOLE_WRITE(platform_console_write)
{
	write(1, buffer, length);
}

PLATFORM_PROCESS_SLEEP(platform_process_sleep)
{
	// NOTE(max): I ignore remaining time
	sleep((unsigned int)seconds); 
}	

PLATFORM_GET_TIME(platform_get_time)
{
	App_time_info result = {};

	time_t t = time(0);	
	struct tm *local_t = localtime(&t);

	result.hour = local_t->tm_hour;
	result.minute = local_t->tm_min;
	result.second = local_t->tm_sec;
	
	return (result);
}

int main(int argc, char *argv[])
{
	// init ncurses
	Platform_callbacks callbacks = {};
	callbacks.platform_process_create = platform_process_create;
	callbacks.platform_console_write = platform_console_write;
	callbacks.platform_process_sleep = platform_process_sleep;
	callbacks.platform_get_time = platform_get_time;

	application_main(&callbacks);
	
	return (0);
}
