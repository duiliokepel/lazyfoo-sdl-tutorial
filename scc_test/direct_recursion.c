#include <stdio.h>

int factorial(int n);

int factorial(int n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

int main(int argc, char *argv[]) {
    int counter;
    for (counter = 0; counter < argc; counter++) {
        printf("argv[%d]=[%s]\n", counter, argv[counter]);
    }

    printf("factorial(5)=%d\n", factorial(5));
    return 0;
}
