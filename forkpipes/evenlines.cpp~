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

void filter_even_lines(int read_end, int write_end) {
    char line[BUFFER_SIZE];
    int line_number = 0;

    while (read(read_end, line, BUFFER_SIZE) > 0) {
        line_number++;
        if (line_number % 2 == 0) { // Send only even lines
            write(write_end, line, strlen(line));
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    int pipe_fd1[2], pipe_fd2[2];
    if (pipe(pipe_fd1) < 0 || pipe(pipe_fd2) < 0) {
        perror("Cannot create pipe");
        exit(1);
    }

    pid_t child_pid1 = fork();
    if (child_pid1 < 0) {
        perror("Cannot create new process");
        exit(1);
    }

    if (child_pid1 == 0) { // První potomek
        close(pipe_fd1[0]); // Zavře čtecí konec první roury
        file_handler(argv[1], pipe_fd1[1]);
        close(pipe_fd1[1]); // Zavře zapisovací konec první roury
        exit(0);
    } else {
        pid_t child_pid2 = fork();
        if (child_pid2 < 0) {
            perror("Cannot create new process");
            exit(1);
        }

        if (child_pid2 == 0) { // Druhý potomek
            close(pipe_fd1[1]); // Zavře zapisovací konec první roury
            close(pipe_fd2[0]); // Zavře čtecí konec druhé roury
            filter_even_lines(pipe_fd1[0], pipe_fd2[1]);
            close(pipe_fd1[0]);
            close(pipe_fd2[1]);
            exit(0);
        } else {
            // Rodičovský proces
            close(pipe_fd1[0]);
            close(pipe_fd1[1]); // Zavře oba konce první roury
            close(pipe_fd2[1]); // Zavře zapisovací konec druhé roury

            char read_buffer[BUFFER_SIZE];
            int bytesRead;
            while ((bytesRead = read(pipe_fd2[0], read_buffer, BUFFER_SIZE - 1)) > 0) {
                read_buffer[bytesRead] = '\0';
                printf("Potomek 2: %s", read_buffer);
            }
            close(pipe_fd2[0]);

            waitpid(child_pid1, NULL, 0);
            waitpid(child_pid2, NULL, 0);
        }
    }

    return 0;
}
