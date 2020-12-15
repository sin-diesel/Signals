#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/time.h>
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

#define DBG(expr) if (DEBUG) {expr;}
#define SLEEP if (SLP) {sleep(1);}


void send_data(char* buf, int buf_size, int pid) {

    union sigval sv;
    sigset_t waitset;
    siginfo_t siginfo;
    struct timespec susp;
    susp.tv_sec = 2;
    susp.tv_nsec = 0;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGUSR2);
    int res = sigprocmask(SIG_BLOCK, &waitset, NULL);
    if (res == -1) {
        fprintf(stderr, "Cannot block signals\n");
        exit(-1);
    }

    int count = 0;
    
    clock_t begin = clock();

    for (int i = 0; i < buf_size; i = i + sizeof(int)) {
        memcpy(&(sv.sival_int), buf + i, sizeof(int));
        sigqueue(pid, SIGUSR1, sv);
        res = sigtimedwait(&waitset, &siginfo, &susp);
        ++count;
    }
    clock_t end = clock();

    clock_t ticks = end - begin;

    long long msec = 1000 * ticks / CLOCKS_PER_SEC;
    printf("Speed: %lld bytes/sec\n", buf_size / msec * 1000);
}

void send_size(int input_size, int pid) {

    union sigval sv;
    sigset_t waitset;
    siginfo_t siginfo;
    struct timespec susp;
    susp.tv_sec = 2;
    susp.tv_nsec = 0;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGUSR2);
    int res = sigprocmask(SIG_BLOCK, &waitset, NULL);
    if (res == -1) {
        fprintf(stderr, "Cannot block signals\n");
        exit(-1);
    }
    
    sv.sival_int = input_size;
    sigqueue(pid, SIGUSR1, sv);
    res = sigtimedwait(&waitset, &siginfo, &susp);
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
    send_data(buf, input_size, pid);

    close(fd);
    return 0;
}