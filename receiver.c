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

#define BYTE_SIZE 8
#define INT_SIZE BYTE_SIZE * 4

#define DEBUG 0

#define DBG(expr) if (DEBUG) {expr}

unsigned count = 0;
unsigned is_read = 0;
unsigned input_size = 0;
char* buf = NULL;
char current_byte = 0;
unsigned current_len = 0;
int fd;

void term_handler(int signal) {
    buf[input_size] = '\0';
    //fprintf(stdout, "%s\n", buf);
    fprintf(stderr,"Input_size: %d\n", input_size);
    int n_write = write(fd, buf, input_size * sizeof(char)); 
    assert(n_write == input_size);
    close(fd);
    exit(0);
}

void usr1_handler(int signal) {
    //DBG(fprintf(stderr, "0");)
    if (is_read == 0) {
        input_size = input_size << 1;
    }

    if (is_read == 1) {
        current_byte = current_byte << 1;
    }

    if (count == INT_SIZE - 1 && is_read == 0) {
        //DBG(fprintf(stderr, "\nSize received: %d\n", input_size);)
        buf = (char*) calloc(input_size, sizeof(char));
        assert(buf != NULL);
        is_read  = 1;
        count = 0;
        return;
    }

    if (count == BYTE_SIZE - 1 && is_read == 1) {
        //DBG(fprintf(stderr, "\nByte received: %c\n", current_byte);)
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

    
    //printf("signal received!\n");
}

void usr2_handler(int signal) {
    //DBG(fprintf(stderr, "1");)
    if (is_read == 0) {
        input_size = input_size << 1;
        input_size++;
    }

    if (is_read == 1) {
        current_byte = current_byte << 1;
        current_byte++;
    }

    if (count == INT_SIZE - 1 && is_read == 0) {
        //DBG(fprintf(stderr, "\nSize received: %d\n", input_size);)
        buf = (char*) calloc(input_size, sizeof(char));
        assert(buf != NULL);
        is_read  = 1;
        count = 0;
        return;
    }

     if (count == BYTE_SIZE - 1 && is_read == 1) {
       //DBG(fprintf(stderr, "\nByte received: %c\n", current_byte);)
       //fprintf(stdout, "%c", current_byte);
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
    //printf("signal received!\n");
}


int main (int argc, char** argv) {

    assert(argc == 2);
    char* file = argv[1];

    fd = open(file, O_WRONLY | O_CREAT, 0666);
    assert(fd > 0);


    int pid = getpid();
    fprintf(stdout, "PID: %d\n", pid);

    signal(SIGUSR1, &usr1_handler);
    signal(SIGUSR2, &usr2_handler);
    signal(SIGTERM, &term_handler);

    while (1) {
    }

    return 0;
}