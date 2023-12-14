#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>


int main (int t_narg, char **t_nargs)
{
   	char c;
   	FILE *input_file;
   	int n = atoi(t_nargs[2]);
	
	if(t_narg != 2)
	{
		printf("Usage: ./mcut 'filename'");
	}
	
   input_file = fopen(t_nargs[1], "r");
   if (input_file == NULL)
   {
      fprintf(stderr, "Error opening input file\n\n");
      exit (1);
   }
	
	while(c != EOF)
	{	
		char line[n];
    	for (int i = 0; i < n; i++)
    	{
        	char c = getc(input_file);
        	line[i] = c;
        	printf("%c", line[i]);
    	}
	}	
	
}

