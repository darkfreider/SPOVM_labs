
#if !defined(_APP_H)
#define _APP_H

struct App_time_info
{
	int hour;
	int minute;
	int second;
};

// RETURN_VALUE:
// 	0: failure
// 	non negative value: success
//
#define PLATFORM_PROCESS_CREATE(name) int name(const char *module_name)
typedef PLATFORM_PROCESS_CREATE(Platform_process_create_type);

#define PLATFORM_CONSOLE_WRITE(name) void name(const char *buffer, int length)
typedef PLATFORM_CONSOLE_WRITE(Platform_console_write_type);

#define PLATFORM_PROCESS_SLEEP(name) void name(int seconds)
typedef PLATFORM_PROCESS_SLEEP(Platform_process_sleep_type);

#define PLATFORM_GET_TIME(name) App_time_info name(void)
typedef PLATFORM_GET_TIME(Platform_get_time_type);

struct Platform_callbacks
{
	Platform_process_create_type *platform_process_create;
	Platform_console_write_type *platform_console_write;
	Platform_process_sleep_type *platform_process_sleep;
	Platform_get_time_type *platform_get_time;
};

#define APPLICATION_MAIN(name) void name(Platform_callbacks *callbacks)

#endif
