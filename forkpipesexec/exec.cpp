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
	if(t_n_arg < 2)
	{
		fprintf(stderr, "Usage: %s <command> [args...]\n", t_args[0]);
	}

	int l_pipe[2];
	
	if (pipe(l_pipe) < 0)
	{
		perror("Cannot create pipe!");
	}

	pid_t l_child = fork();
	
	if(l_child < 0)
	{
		perror("Cannot create new process!");
		exit(1);
	}
	
	//child process
	if(l_child == 0)
	{
		//spustit pomoci exec* specifikovany prikaz predany pres argumenty programu
		//char *args[] = ("ls", -"-l", "/", NULL);
		//execvp("ls",args);
		execvp(t_args[1], &t_args[1]);
		perror("execvp failed");	
		//standardni vystup programu presmerovat do roury
		close(l_pipe[0]); //cteci konec roury
		dup2(l_pipe[1], STDOUT_FILENO);
		//write(l_pipe[1], );
		close(l_pipe[1]); //uz nepotrebujeme zapisovaci konec
	}
	//parrent process
	else
	{
		//vypise data zaslana do pipy na standardni vystup
		int bytes_read;
		char buffer[BUFSIZE];
		close(l_pipe[1]); //zapisovaci konec roury
		while((bytes_read = read(l_pipe[0], buffer, BUFSIZE - 1)) > 0)
		{
			buffer[bytes_read] = '\0'; //pridani nuloveho znaku pro ukonceni retezce
			printf("Parrent accepted: %s\n", buffer);
		}
		close(l_pipe[0]); //uz nepotrebuheme cteci konec
		//pockat korekektne na ukonceni potomka
		int status;
		waitpid(l_child, &status, 0); //nebo wait(NULLPTR);
		if(WIFEXITED(status))
		{
			printf("Child ended with status code %d\n", WEXITSTATUS(status));
		}
	}
}