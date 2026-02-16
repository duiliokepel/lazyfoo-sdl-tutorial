#include <setjmp.h>
#include <stdio.h>

jmp_buf buf;

void func(void);
int main(int argc, char* argv[]);

void func(void) {
    printf("Execute 2\n");

    longjmp(buf, 1);

    printf("Never execute\n");
}

int main(int argc, char* argv[]) {
    int counter;
    for (counter = 0; counter < argc; counter++) {
        printf("argv[%d]=[%s]\n", counter, argv[counter]);
    }

    if (setjmp(buf)) {
        printf("Execute 3\n");
    } else {
        printf("Execute 1\n");
        func();
    }

    return 0;
}
