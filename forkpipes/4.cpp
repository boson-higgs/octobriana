#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_children>\n", argv[0]);
        return 1;
    }

    int num_children = atoi(argv[1]);
    int pipes[num_children][2];
    pid_t pids[num_children];

    for (int i = 0; i < num_children; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(1);
        }

        if (pids[i] == 0) { // Potomek
            close(pipes[i][0]); // Zavře čtecí konec roury

            srand(time(NULL) ^ (getpid()<<16));
            int wait_time = rand() % 5 + 1; // Náhodná doba čekání 1-5 sekund
            sleep(wait_time);

            printf("Potomek PID %d čekal %d sekund\n", getpid(), wait_time);
            write(pipes[i][1], &wait_time, sizeof(wait_time));
            close(pipes[i][1]);

            exit(0);
        }
    }

    int max_wait_time = 0;
    for (int i = 0; i < num_children; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        close(pipes[i][1]); // Zavře zapisovací konec roury
        int child_wait_time;
        read(pipes[i][0], &child_wait_time, sizeof(child_wait_time));
        close(pipes[i][0]);

        if (child_wait_time > max_wait_time) {
            max_wait_time = child_wait_time;
        }
    }

    printf("Nejdelší doba čekání byla %d sekund\n", max_wait_time);
    return 0;
}
