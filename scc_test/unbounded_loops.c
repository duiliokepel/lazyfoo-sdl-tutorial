#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "../src/assert.h"

int main(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    int counter;
    unsigned long loop_counter = 0;
    unsigned long max_iterations = 10;
    bool bound_tripped = false;
    int very_long_named_argument_a = 0;
    int very_long_named_argument_b = 0;
    int very_long_named_argument_c = 0;
    int very_long_named_argument_d = 0;
    int very_long_named_argument_e = 0;

    loop_counter = 0;
    for (counter = 0;
         LOOPBOUND(counter < argc, loop_counter, max_iterations, NULL);
         counter++) {
        printf("argv[%d]=[%s]\n", counter, argv[counter]);
    }

    // Must pass tests:
    loop_counter = 0;
    for (; LOOPBOUND(1, loop_counter, max_iterations, NULL);) {
        printf("Bounded for(;;) loop - loop_counter=[%lu]\n", loop_counter);
    }

    loop_counter = 0;
    while (LOOPBOUND(1, loop_counter, max_iterations, NULL)) {
        printf("Bounded while(1) loop - loop_counter=[%lu]\n", loop_counter);
    }

    loop_counter = 0;
    while (LOOPBOUND(true, loop_counter, max_iterations, NULL)) {
        printf("Bounded while(true) loop - loop_counter=[%lu]\n", loop_counter);
    }

    loop_counter = 0;
    do {
        printf("Bounded do while(1) loop - loop_counter=[%lu]\n", loop_counter);
    } while (LOOPBOUND(1, loop_counter, max_iterations, NULL));

    loop_counter = 0;
    do {
        printf("Bounded do while(true) loop - loop_counter=[%lu]\n",
               loop_counter);
    } while (LOOPBOUND(true, loop_counter, max_iterations, NULL));

    loop_counter = 0;
    bound_tripped = false;
    while (LOOPBOUND(very_long_named_argument_a < INT_MAX &&
                         very_long_named_argument_b < INT_MAX &&
                         very_long_named_argument_c < INT_MAX &&
                         very_long_named_argument_d < INT_MAX &&
                         very_long_named_argument_e < INT_MAX,
                     loop_counter, max_iterations, &bound_tripped)) {
        very_long_named_argument_a++;
        very_long_named_argument_b++;
        very_long_named_argument_c++;
        very_long_named_argument_d++;
        very_long_named_argument_e++;
        printf("Bounded multiline condition while loop - loop_counter=[%lu]\n",
               loop_counter);
    };
    ASSERT (bound_tripped == false, "Loop exceeded max iterations") {
        return 0;
    }

    // Must fail tests
    for (;;) { printf("Infinite loop\n"); }

    while (1) { printf("Infinite loop\n"); }

    while (true) { printf("Infinite loop\n"); }

    do { printf("Infinite loop\n"); } while (1);

    do { printf("Infinite loop\n"); } while (true);

    // multiline condition
    while (very_long_named_argument_a < INT_MAX &&
           very_long_named_argument_b < INT_MAX &&
           very_long_named_argument_c < INT_MAX &&
           very_long_named_argument_d < INT_MAX &&
           very_long_named_argument_e < INT_MAX) {
        very_long_named_argument_a++;
        very_long_named_argument_b++;
        very_long_named_argument_c++;
        very_long_named_argument_d++;
        very_long_named_argument_e++;
        break;
    };

    return 0;
}
