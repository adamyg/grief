#pragma once
//
//  getopt/getopt_long
//

#ifdef __cplusplus
extern "C" {
#endif

extern int opterr;	    /* if error message should be printed */
extern int optind;	    /* index into parent argv vector */
extern int optopt;	    /* character checked for validity */
extern int optreset;	    /* reset getopt */
extern const char *optarg;  /* argument associated with option */

struct option {
        const char *name;
        int has_arg;
#define no_argument         0
#define required_argument   1
#define optional_argument   2

        int *flag;
        int val;
};

int getopt(int nargc, const char * const nargv[], const char *ostr);
int getopt_long(int nargc, const char * const * nargv, const char * options, const struct option * long_options, int * index, int retval);

#ifdef __cplusplus
}
#endif

/*end*/
