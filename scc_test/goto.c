#include <stdio.h>

int main(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    int counter;
    int var_gotoX = 1;
    for (counter = 0; counter < argc; counter++) {
        printf("argv[%d]=[%s]\n", counter, argv[counter]);
    }

    printf("Execute 1\n");
    goto label;

    printf("Never execute\n");

label:
    printf("Execute 2\n");
    printf("var_gotoX = %d\n", var_gotoX);

    return 0;
}
