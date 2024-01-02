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

#define BUFFER_SIZE 1024

std::ostringstream g_buffer;
char g_read_buffer[BUFFER_SIZE];

void file_handler(std::string t_file)
{
    std::string l_line;
    int l_line_number = 1;
    std::ifstream l_file(t_file);
    if(!l_file)
    {
       perror("Cannot open file");
       exit (1);
    }
    while (getline(l_file, l_line))
    {
       g_buffer << l_line_number << ":" << l_line << '\n';
       l_line_number++;
    }
    l_file.close();
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    int l_pipe[2];

     if(pipe(l_pipe) < 0)
    {
       perror("Cannot create pipe");
       exit(1);
    }

    pid_t l_child = fork(); //pid

    int l_status;

    if( l_child < 0)
    {
       perror("Cannot create new process!"); 
       exit(1);
    }

    if (l_child != 0) //parent process
    {
      //vypisuje prijate data a pote si prevezme status potomka pomoci waitpid
      close(l_pipe[1]); //zavrit zapisovaci proces
      int bytes_read;
      while( (bytes_read = read(l_pipe[0], g_read_buffer, BUFFER_SIZE - 1)) > 0 )
      {
         g_read_buffer[bytes_read] = '/0';
         std::cout << g_read_buffer;
      }
      close(l_pipe[0]); //zavrit cteci proces
      waitpid(l_child, &l_status, 0);
    }
    else //child process
    {
      //precte soubor a preda ho rodici s ocislovnymi radky
      file_handler(argv[1]);
      close(l_pipe[0]); //zavrit cteci proces
      write(l_pipe[1], g_buffer.str().c_str(), g_buffer.str().size());
      close(l_pipe[1]); //zavrit zapisovaci proces
      exit(0);
    }

    return 0;
}

