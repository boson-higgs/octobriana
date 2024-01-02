#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 1024

void file_handler(const char* filename, int pipe_write_end) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        exit(1);
    }

    char line[BUFFER_SIZE];
    int line_number = 1;
    while (fgets(line, sizeof(line), file)) {
        char formatted_line[BUFFER_SIZE];
        snprintf(formatted_line, BUFFER_SIZE, "%d: %s", line_number, line);
        write(pipe_write_end, formatted_line, strlen(formatted_line));
        line_number++;
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_name1> [file_name2] ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) < 0) {
            perror("Cannot create pipe");
            exit(1);
        }

        pid_t child_pid = fork();
        if (child_pid < 0) {
            perror("Cannot create new process");
            exit(1);
        }

        if (child_pid == 0) { // Potomek
            close(pipe_fd[0]); // Zavře čtecí konec roury
            file_handler(argv[i], pipe_fd[1]);
            close(pipe_fd[1]); // Zavře zapisovací konec roury
            exit(0);
        } else { // Rodič
            close(pipe_fd[1]); // Zavře zapisovací konec roury

            char read_buffer[BUFFER_SIZE];
            int bytesRead;
            while ((bytesRead = read(pipe_fd[0], read_buffer, BUFFER_SIZE - 1)) > 0) {
                read_buffer[bytesRead] = '\0';
                printf("%s", read_buffer);
            }
            close(pipe_fd[0]);

            waitpid(child_pid, NULL, 0);
        }
    }

    return 0;
}
