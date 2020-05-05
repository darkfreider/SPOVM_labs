

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

int main(void)
{
	
	/*
	 * init 3 semaphores (mutex, str_present, str_not_present)
	 * create child process that will consume string and then print them on a screen
	 * start parent process loop of string consumption and dispatch
	 * */

	sem_unlink("/binary_sem");
	sem_unlink("/str_present");
	sem_unlink("/str_not_present");
	
	sem_t *mutex = sem_open("/binary_sem", O_CREAT | O_RDWR, 0664, 1);
	if (mutex == SEM_FAILED)
	{
		exit(-1);
	}
	
	sem_t *str_present = sem_open("/str_present", O_CREAT | O_RDWR, 0664, 0);
	if (str_present == SEM_FAILED)
	{
		sem_close(mutex);
		exit(-1);	
	}
	
	sem_t *str_not_present = sem_open("/str_not_present", O_CREAT | O_RDWR, 0664, 1);
	if (str_not_present == SEM_FAILED)
	{
		sem_close(mutex);
		sem_close(str_present);
		exit(-1);	
	}
	
	

	int fd[2];
	pipe(fd);

	int fork_res = fork();
	if (fork_res == 0)
	{
		sem_t *mutex = sem_open("/binary_sem", O_RDWR);	
		if (mutex == SEM_FAILED)
		{
			printf("child sem f\n");
			exit(-1);
		}
		sem_t *str_present = sem_open("/str_present", O_RDWR); 	
		if (str_present == SEM_FAILED)
		{
			printf("child sem f\n");
			sem_close(mutex);
			exit(-1);	
		}
		sem_t *str_not_present = sem_open("/str_not_present", O_RDWR);	
		if (str_not_present == SEM_FAILED)
		{
			printf("child sem f\n");	
			sem_close(mutex);
			sem_close(str_present);
			exit(-1);	
		}
	
		// child
		close(fd[PIPE_WRITE]);

		char text_buffer[512];
		for (;;)
		{
			sem_wait(str_present);
			sem_wait(mutex);

			int len = 0;
			read(fd[PIPE_READ], &len, sizeof(int));
			read(fd[PIPE_READ], text_buffer, len + 1);
			printf("%d: %s", len, text_buffer);	

			sem_post(mutex);
			sem_post(str_not_present);
		}
	}
	else if (fork_res > 0)
	{
		
		// parent
		close(fd[PIPE_READ]);
		
		char text_buffer[512];	
		for (;;)
		{
			char *res = fgets(text_buffer, sizeof(text_buffer), stdin);	
			if (!res)
			{
				break;
			}

		
			sem_wait(str_not_present);	
			sem_wait(mutex);
	
			// write to pipe
			int len = strlen(text_buffer);
			write(fd[PIPE_WRITE], &len, sizeof(int));	
			write(fd[PIPE_WRITE], text_buffer, len + 1);
			

			sem_post(mutex);
			sem_post(str_present);	
		}	
	}
	else
	{
		printf("can't fork\n");	
	}

	

	return (0);
}




















