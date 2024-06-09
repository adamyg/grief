#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libguess.h>

char buf[1024 * 1024];

int main(int argc, char **argv)
{
    // char buf[1024 * 1024];
    FILE *fp;

    if (argc != 2)
        return EXIT_FAILURE;

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "%s: cannot open file '%s'\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }

    while (fgets(buf, 1024 * 1024 - 1, fp))
    {
        printf("length = %zu\n", strlen(buf));
        printf("jp = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_JP));
        printf("tw = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_TW));
        printf("cn = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_CN));
        printf("kr = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_KR));
        printf("ru = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_RU));
        printf("ar = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_AR));
        printf("tr = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_TR));
        printf("gr = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_GR));
        printf("hw = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_HW));
        printf("pl = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_PL));
        printf("bl = %s\n", libguess_determine_encoding(buf, strlen(buf), GUESS_REGION_BL));
    }
    fclose(fp);

    return EXIT_SUCCESS;
}
