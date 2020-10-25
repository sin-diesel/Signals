#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define DEBUG 0

#define DBG(expr) if (DEBUG) {expr}

#define NANO_SLEEP 130000



void to_bits(char* buf, int buf_size, int pid) {
    time_t begin;
    time(&begin);

    for (int i = 0; i < buf_size; ++i) {
        char byte = buf[i];
        for (unsigned j = 0; j < sizeof(char) * 8; ++j) {
            unsigned mask = 1 << (sizeof(char) * 8 - 1 - j);
            char bit = (mask & byte) >> (sizeof(char) * 8 - 1 - j);
            DBG(fprintf(stderr, "%d", bit);)
            struct timespec time;
            time.tv_nsec = NANO_SLEEP;
            time.tv_sec = 0;
            struct timespec rem;
            nanosleep(&time, &rem);
            //sleep(1);
            if (bit == 0) {
                kill(pid, SIGUSR1);
            } else kill(pid, SIGUSR2);
        }
        DBG(fprintf(stderr, " ");)
    }
    kill(pid, SIGTERM);
    time_t end;
    time(&end);

    //fprintf(stderr, "buf_size %d\n", buf_size);
    unsigned speed = buf_size / difftime(end, begin);

    printf("Speed: %u Bytes/sec\n", speed);
    // clock_t end = clock();
    // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    // unsigned speed = buf_size / time_spent;
    // printf("Speed: %u Bytes/sec\n", speed);
}

// void send_size_queued(int size, int pid) {
//     union sigval data;
//     data.sival_int = size;
//     sigqueue(pid, SIGUSR1, data);
//}

// void send_queued(char* buf, int buf_size, int pid) {
//     for (int i = 0; i < buf_size; ++i) {
//         char byte = buf[i];
//         union sigval data;
//         data.sival_int = byte;
//         sigqueue(pid, SIGUSR1, data);
//     }
// }

void send_size(int input_size, int pid) {
    for (unsigned j = 0; j < sizeof(int) * 8; ++j) {
            unsigned mask = 1 << (sizeof(int) * 8 - 1 - j);
            char bit = (mask & input_size) >> (sizeof(int) * 8 - 1 - j);
            DBG(fprintf(stderr, "%d", bit);)
            struct timespec time;
            time.tv_nsec = NANO_SLEEP;
            time.tv_sec = 0;
            struct timespec rem;
            nanosleep(&time, &rem);
            //sleep(1);
            if (bit == 0) {
                kill(pid, SIGUSR1);
                //fprintf(stderr, "Sending signal 0!\n");
            } else {
                kill(pid, SIGUSR2);
                //fprintf(stderr, "Sending signal 1!\n");
            }
        }
        DBG(fprintf(stderr, "\n");)
}
int main (int argc, char** argv) {

    assert(argc == 3);

    char* file = argv[1];
    int pid = strtol(argv[2], &argv[2], 10);
    fprintf(stdout, "Receiver pid: %d\n", pid);

    int fd = open(file, O_RDONLY);
    assert(fd > 0);
    struct stat input;
    fstat(fd, &input);
    int input_size = input.st_size;

    char* buf = (char*) calloc(input_size, sizeof(char));
    assert(buf != NULL);
    int n_read = read(fd, buf, input_size);
    fprintf(stdout, "Read in bytes: %d\n", n_read);
    assert(n_read == input_size);
    buf[input_size] = '\0';

    //fprintf(stderr, "Buf read: %s\n", buf);

    //send_size_queued(input_size, pid);
    send_size(input_size, pid);
    
    to_bits(buf, input_size, pid);

    close(fd);
    return 0;
}