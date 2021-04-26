/* sample2.c - a sample program for shm library
 * Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp> 2003-11-05, 2006-07-14
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "shm.h"

#define DEFAULT_KEY	123

typedef struct _SAMPLE_STRACTURE_ {
	long x;
	long y;
	long z;
	/* You shuldn't use any pointer vailables.
	 */
} SAMP;


void
writing(unsigned int key)
{
	SAMP *shared_mem;
	int i, j;

	/* You must use the pointer variables.
	   The shm_get_buf returns a pointer of shared memory.
	*/
	/* You shuld use the sizeof(), which counts a size of a variable.
	*/ 
	shared_mem = (SAMP *)shm_get_buf(key, sizeof(SAMP));

	shared_mem->x = shared_mem->y = shared_mem->z = 0L;
	
	for (i = 0; i < 100; i++) {
		j = rand() / (RAND_MAX / 3);
		if (j  == 0)
			shared_mem->x += 1;
		else if (j == 1)
			shared_mem->y += 1;
		else if (j == 2)
			shared_mem->z += 1;

		printf("send: shared_mem(%d)->x, y, z = %ld, %ld, %ld\n", 
		       key, shared_mem->x, shared_mem->y, shared_mem->z);
		sleep(1);
	}
}


void
reading(unsigned int key)
{
	SAMP *shared_mem;
	int i;

	shared_mem = (SAMP *)shm_get_buf(key, sizeof(SAMP));

	for (i = 0; i < 10; i++) {
		printf("send: shared_mem(%d)->x, y, z = %ld, %ld, %ld\n", 
		       key, shared_mem->x, shared_mem->y, shared_mem->z);
		usleep(500000); /* 0.5[sec]*/
	}
}


void
usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s {-w|-r} {-k num|-K num}", prog_name);
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t-w\t\texecute the writing\n");
	fprintf(stderr, "\t-r\t\texecute the reading\n");
	fprintf(stderr, "\t-k num\t\tset the key number\n");
	fprintf(stderr, "\t-K num\t\tkill shared memory\n");
	exit(1);
}


int
main(int argc, char **argv)
{
	unsigned int key = DEFAULT_KEY;
	int opt, flag = 0;

	if (argc == 1) {
		usage(argv[0]);
	}

	do {
		opt = getopt(argc, argv, "wrk:Kh?");
		switch (opt) {
		case 'w':
			flag = 1;
			break;
		case 'r':
			flag = 2;
			break;
		case 'k':
			key = atoi(optarg);
			break;
		case 'K':
			shm_rm_key(key);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
		}
	} while (opt > 0);


	switch (flag) {
	case 1:
		writing(key);
		break;
	case 2:
		reading(key);
		break;
	default:
		break;
	}		

	return 0;
}
