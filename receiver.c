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


//-------------------- for debugging
#define BYTE_SIZE 8
#define INT_SIZE BYTE_SIZE * 4

#define DEBUG 0
#define SLP 0

#define DBG(expr) if (DEBUG) {expr;}
#define SLEEP if (SLP) {sleep(1);}

int packs = 0;
unsigned count = 0;
unsigned is_read = 0;
int current_size;
unsigned input_size = -1;
char* buf = NULL;
char current_byte = 0;
unsigned current_len = 0;
int fd = 0;

void handler(int bit) {

    DBG(fprintf(stderr, "0\n");)
    if (is_read == 0) {
        input_size = input_size << 1;
        if (bit == 1) {
            input_size++;
        }
    }

    if (is_read == 1) {
        current_byte = current_byte << 1;
        if (bit == 1) {
            current_byte++;
        }
    }

    if (count == INT_SIZE - 1 && is_read == 0) {
        DBG(fprintf(stderr, "\nSize received: %d\n", input_size))
        buf = (char*) calloc(input_size, sizeof(char));
        assert(buf != NULL);
        is_read  = 1;
        count = 0;
        return;
    }

    if (count == BYTE_SIZE - 1 && is_read == 1) {
        DBG(fprintf(stderr, "\nByte received: %c\n", current_byte))
        buf[current_len] = current_byte;
        current_len++;
        count = 0;
        return;
    }

    if (count < INT_SIZE && is_read == 0) {
         count++;
    }

    if (count < BYTE_SIZE && is_read == 1) {
         count++;
    }

}

static void handler_1(int sig, siginfo_t *si, void *ucontext) {
    if (input_size == -1) {
        input_size = si->si_value.sival_int;
        kill(si->si_pid, SIGUSR2);
    } else { 
        if (buf == NULL) {
            buf = (char*) calloc(input_size, sizeof(char));
            assert(buf);
        }
        memcpy(buf + packs * sizeof(int), &(si->si_value.sival_int), sizeof(int));
        ++packs;
        current_size += sizeof(int);
        kill(si->si_pid, SIGUSR2);
    }
}


int main (int argc, char** argv) {

    assert(argc == 2);
    char* file = argv[1];

    fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    assert(fd > 0);

    int pid = getpid();
    fprintf(stdout, "PID: %d\n", pid);

    int res = -1;

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler_1;
    sa.sa_flags = SA_SIGINFO; /* Important. */

    sigaction(SIGUSR1, &sa, NULL);
    
    while (1) {
        if (current_size >= input_size) {
            buf[input_size] = '\0';
            fprintf(stdout,"Input_size: %d\n", input_size);
            fprintf(stdout,"End of transmission.\n");
            int n_write = write(fd, buf, input_size * sizeof(char)); 
            assert(n_write == input_size);
            free(buf);
            close(fd);
            exit(0);
        }
    }
    return 0;
}