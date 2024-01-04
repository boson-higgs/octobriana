#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ostream>

#define BUFSIZE 4096

int main(int t_n_arg, char *t_args[])
{
	if(t_n_arg < 3)
	{
		fprintf(stderr, "Usage: %s <command> [args...]\n", t_args[0]);
	}

	int l_pipe[2], l_pipe2[2];
	
	if (pipe(l_pipe) < 0 || pipe(l_pipe2) < 0)
	{
		perror("Cannot create pipe!");
	}

	pid_t l_child = fork();
	
	if(l_child < 0)
	{
		perror("Cannot create new process!");
		exit(1);
	}

	//first child process
	if(l_child == 0)
	{
		//spustit pomoci exec* specifikovany prikaz predany pres argumenty programu
		//char *args[] = ("ls", -"-l", "/", NULL);
		//execvp("ls",args);
		t_args[t_n_arg - 1] = NULL;
		execvp(t_args[1], &t_args[1]);
		perror("execvp failed");	
		//standardni vystup programu presmerovat do roury
		close(l_pipe[0]); //cteci konec roury
		dup2(l_pipe[1], STDOUT_FILENO);
		//write(l_pipe[1], );
		//close(l_pipe[1]); //uz nepotrebujeme zapisovaci konec
	}
	//parrent process
	else
	{
		close(l_pipe[1]); //uz nepotrebujeme zapisovaci konec


		pid_t l_child2 = fork();
	
		if(l_child2 < 0)
		{
			perror("Cannot create child2 process!");
			exit(1);
		}
		//second child process
		if(l_child2 == 0)
		{
			//prijmout data prvniho potomka
			close(l_pipe2[0]); //cteci konec druhe roury
			dup2(l_pipe[0], STDIN_FILENO);
			close(l_pipe[0]); //uz nepotrebujeme cteci konec
			//spustit grep s poslednim argumentem nad prijatymi daty z prvniho potomka
			execlp("grep", "grep", t_args[t_n_arg - 1], (char *)NULL);
			perror("execvp failed");
			//close(l_pipe[0]);
			dup2(l_pipe2[1], STDOUT_FILENO);
			close(l_pipe2[1]);
		}
		close(l_pipe[0]);
		close(l_pipe2[1]);

		//vypise data zaslana do pipy na standardni vystup
		int bytes_read;
		char buffer[BUFSIZE];
		close(l_pipe2[1]); //zapisovaci konec roury
		close(l_pipe[0]); //cteci konec druhe roury
		while((bytes_read = read(l_pipe2[0], buffer, BUFSIZE - 1)) > 0)
		{
			buffer[bytes_read] = '\0'; //pridani nuloveho znaku pro ukonceni retezce
			printf("Parent accepted: %s\n", buffer);
		}
		close(l_pipe2[0]); //uz nepotrebujeme cteci konec	
		//pockat korekektne na ukonceni obou potomku
		int status;
		waitpid(l_child, &status, 0); //nebo wait(NULLPTR);
		waitpid(l_child2, &status, 0);
		if(WIFEXITED(status))
		{
			printf("Child ended with status code %d\n", WEXITSTATUS(status));
		}
	}
}