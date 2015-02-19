//----------------------------------------------------------------------
// A complex algorithm, with the aim to give a greeting
//----------------------------------------------------------------------

#include <unistd.h>
#include "hello.h"

int
main(void)
{
    int opt, number_of_greetings = DEFAULT_GREETINGS;

    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n':
            number_of_greetings = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-n greetings]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    while (number_of_greetings-- > 0) {
        printf("hello world\n");
    }
    return 0;
}

