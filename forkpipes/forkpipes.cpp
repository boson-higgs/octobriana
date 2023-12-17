#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
int main(int argc, char** argv)
{
	int num = atoi(argv[1]);
	
	pid_t pids[num];
	pids[num] = fork();
	
	int fds[num][2];
	
	int times[num];
	int status;
  	
  	for(int i = 0; i < num; ++i)
  	{
  		pipe(fds[i]);
  		
  		if (pids[i] == 0)
  		{
  			close(fds[i][0]);
  			
  			int time = rand();
  			sleep(time);
  			print("%d ", time);
  			print("%d\n", pids[i]);
  			
  			write(fds[i][1], &time, 1);
  			close(fds[i][1]);
  			sleep(time);
  			exit(0);
  		}
  		
  		else if(pids[i] > 0)
  		{
  			close(fds[i][1]);
  			
  			read(fds[0],&times[i], i);
  			
  			close(fds[i][0]);
  			waitpid(pids[i], &status, 0);
  			
  			for(int j = 0; j < num; ++j)
  			{
  				if(times[0] < times[j])
  				{
  					times[0] = times[j];
  				}
  			}
  			
  			printf("%d\n", times[0]);
  		}
  	}
  		
	
	return 0;
}
