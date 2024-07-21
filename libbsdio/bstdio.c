//
//	BSD stdio test application
//

#undef __BSTDIO_INTERNAL
#include <bstdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void t1(void);
static void t2(void);
static void t3(void);


int
main(int argc, char **argv)
{
	t1();
	t2();
	t3();
	return 0;
}


static void
t1(void)
{
#if defined(__clang__)
#pragma clang diagnostic ignored "-Winvalid-source-encoding"
#endif
	static const unsigned char s1[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZéáúõûóüöíÉÁÕÚÖÜÓÛÍ";
	char b1[512] = {0};
	BFILE *f1;

	if (NULL == (f1 = bfopen("testfile1.txt", "w"))) {
		perror("test1, write open");

	} else {
		assert(bfileno(f1) >= 0);
		(void) bfwrite(s1, 1, sizeof(s1), f1);
		bfclose(f1);
	}

	if (NULL == (f1 = bfopen("testfile1.txt", "r"))) {
		perror("test1, read open");

	} else {
		const int r1 = bfread(b1, 1, sizeof(b1), f1);

		if (r1 != sizeof(s1)) {
			printf("test1, read size\n");
		}

		if (0 != memcmp(b1, s1, sizeof(s1))) {
			printf("test1, data incorrect\n");
		}

		bfclose(f1);
	}
}

static void
t2(void)
{
	static const char s2[] =
		"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char b1[512] = {0};
	BFILE *f2;

	if (NULL == (f2 = bfopen("testfile1.txt", "r+"))) {
		perror("test2, write open");

	} else if (EOF == bfseek(f2, 100, SEEK_SET)) {
		perror("test2, seek2");

	} else {
		(void) bfwrite(s2, 1, sizeof(s2), f2);
		bfclose(f2);
	}

	if (NULL == (f2 = bfopen("testfile1.txt", "r"))) {
		perror("test1, read open");

	} else if (EOF == bfseek(f2, 100, SEEK_CUR)) {
		perror("test2, seek2");

	} else {
		const int r1 = bfread(b1, 1, sizeof(b1), f2);

		if (r1 != sizeof(s2)) {
			printf("test2, read size\n");
		}

		if (0 != memcmp(b1, s2, sizeof(s2))) {
			printf("test2, data incorrect\n");
		}

		bfclose(f2);
	}
}


static void
t3print(BFILE *f3, char ch, double base)
{
	char buf[64];

	sprintf(buf, "%%%c\n", ch);
	bfprintf(f3, buf, base * 1);

	sprintf(buf, "%%8%c\n", ch);
	bfprintf(f3, buf, base * 10.10);

	sprintf(buf, "%%8.8%c\n", ch);
	bfprintf(f3, buf, base * 100.10);

	sprintf(buf, "%%#8.8%c\n", ch);
	bfprintf(f3, buf, base * 1000.10);

	sprintf(buf, "%%+8.8%c\n", ch);
	bfprintf(f3, buf, base * 10000,10);

	sprintf(buf, "%% 8.8%c\n", ch);
	bfprintf(f3, buf, base * 100000.10);
}


static void
t3(void)
{
	BFILE *f3;
	int r;

	if (NULL == (f3 = bfopen("testfile3.txt", "w+"))) {
		perror("test3, write open");

	} else if (16 != (r = bfprintf(f3, "hello %s %d\n", "world", 100))) {
		printf("test3, printf : %d\n", r);

	} else {
		t3print(f3, 'e', 1);
		t3print(f3, 'E', 2);
		t3print(f3, 'f', 3);
		t3print(f3, 'g', 4);
		t3print(f3, 'G', 5);
		bfclose(f3);
	}
}

/*end*/
