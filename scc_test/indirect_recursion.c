#include <stdio.h>

void function1(int n);
void function2(int n);

void function1(int n) {
    if (n <= 0) { return; }
    function2(n - 1);
    return;
}

void function2(int n) {
    if (n <= 0) { return; }
    function1(n - 1);
    return;
}

int main(int argc, char *argv[]) {
    int counter;
    for (counter = 0; counter < argc; counter++) {
        printf("argv[%d]=[%s]\n", counter, argv[counter]);
    }

    function1(5);
    printf("done\n");

    return 0;
}
