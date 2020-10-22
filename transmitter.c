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

#ifdef replace
#define 1 SIGUSR1
#define 2 SIGUSR2
#endif

void to_bits(char* buf, int buf_size, int pid) {
    for (int i = 0; i < buf_size; ++i) {
        char byte = buf[i];
        for (unsigned j = 0; j < sizeof(char) * 8; ++j) {
            unsigned mask = 1 << j;
            char bit = (mask & byte) >> j;
            fprintf(stderr, "%d", bit);
            
        }
        fprintf(stderr, " ");
    }
}

void unit_tests() {
    char buf[] = "Test";
}

int main (int argc, char** argv) {

    assert(argc == 3);

    char* file = argv[1];
    char* pid = argv[2];

    int fd = open(file, O_RDONLY);
    assert(fd > 0);
    struct stat input;
    fstat(fd, &input);
    int input_size = input.st_size;

    char* buf = (char*) calloc(input_size, sizeof(char));
    assert(buf != NULL);
    int n_read = read(fd, buf, input_size);
    fprintf(stderr, "Read in bytes: %d\n", n_read);
    assert(n_read == input_size);
    buf[input_size] = '\0';

    fprintf(stderr, "Buf read: %s\n", buf);
    to_bits(buf, input_size, pid);
    close(fd);
    return 0;
}