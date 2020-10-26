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
#define SLP 0

#define DBG(expr) if (DEBUG) {expr}
#define SLEEP if (SLP) {sleep(1);}



void to_bits(char* buf, int buf_size, int pid) {

    time_t start;
    time(&start);

    int res = -1;
    sigset_t waitset;
    siginfo_t siginfo;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGUSR1);
    sigaddset(&waitset, SIGUSR2);
    res = sigprocmask(SIG_BLOCK, &waitset, NULL);
    if (res == -1) {
        fprintf(stderr, "Cannot block signals\n");
        exit(-1);
    }

    for (int i = 0; i < buf_size; ++i) {
        char byte = buf[i];
        for (unsigned j = 0; j < sizeof(char) * 8; ++j) {

            unsigned mask = 1 << (sizeof(char) * 8 - 1 - j);
            char bit = (mask & byte) >> (sizeof(char) * 8 - 1 - j);

            SLEEP
            if (bit == 0) {
                res = kill(pid, SIGUSR1);
                assert(res != -1);
                DBG(fprintf(stderr, "Sending signal 0!\n");)
            } else {
                res = kill(pid, SIGUSR2);
                assert(res != -1);
                DBG(fprintf(stderr, "Sending signal 1!\n");)
            }
             res = sigwaitinfo(&waitset, &siginfo);
             assert(res != -1);
             DBG(fprintf(stderr, "Received responce!\n");)
        }
    }

    DBG(fprintf(stderr, "\n");)
    kill(pid, SIGTERM);

    time_t end;
    time(&end);

    unsigned speed = buf_size / difftime(end, start);
    fprintf(stdout, "Speed: %u Bytes/sec\n", speed);
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

    int res = -1;
    sigset_t waitset;
    siginfo_t siginfo;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGUSR1);
    sigaddset(&waitset, SIGUSR2);
    res = sigprocmask(SIG_BLOCK, &waitset, NULL);
    if (res == -1) {
        fprintf(stderr, "Cannot block signals\n");
        exit(-1);
    }
    
    for (unsigned j = 0; j < sizeof(int) * 8; ++j) {

        unsigned mask = 1 << (sizeof(int) * 8 - 1 - j);
        char bit = (mask & input_size) >> (sizeof(int) * 8 - 1 - j);
        DBG(fprintf(stderr, "%d", bit);)
        
        SLEEP
        if (bit == 0) {
            res = kill(pid, SIGUSR1);
            assert(res != -1);
            DBG(fprintf(stderr, "Sending signal 0!\n");)
        } else {
            res = kill(pid, SIGUSR2);
            assert(res != -1);
            DBG(fprintf(stderr, "Sending signal 1!\n");)
        }
            res = sigwaitinfo(&waitset, &siginfo);
            assert(res != -1);
            DBG(fprintf(stderr, "Received responce!\n");)
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
    assert(input_size > 0);

    char* buf = (char*) calloc(input_size, sizeof(char));
    assert(buf != NULL);
    int n_read = read(fd, buf, input_size);
    fprintf(stdout, "Read in bytes: %d\n", n_read);
    assert(n_read == input_size);
    buf[input_size] = '\0';

    send_size(input_size, pid);
    to_bits(buf, input_size, pid);

    close(fd);
    return 0;
}