#include <stdbool.h>
#include <stdio.h>

int main(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    int counter;
    for (counter = 0; counter < argc; counter++) {
        printf("argv[%d]=[%s]\n", counter, argv[counter]);
    }

    for (;;) { printf("Infinite loop\n"); }

    while (1) { printf("Infinite loop\n"); }

    while (true) { printf("Infinite loop\n"); }

    do { printf("Infinite loop\n"); } while (1);

    do { printf("Infinite loop\n"); } while (true);

    return 0;
}
