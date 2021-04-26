/* sample.c - a sample program for shm library
 * Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>, 2004-10-08, 2006-07-19
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "shm.h"

#define DEFAULT_KEY	123

void
writing(unsigned int key)
{
	int *shared_mem, i;

	/* You must use the pointer variables.
	   The shm_get_buf returns a pointer of shared memory.
	*/
	/* You shuld use the sizeof(), which counts a size of a variable.
	*/ 
	shared_mem = (int *)shm_get_buf(key, sizeof(int));
	shm_write_info(key, "this is sample1"); 

	for (i = 0; i < 10; i++) {
		*shared_mem = i;
		printf("send: shared_mem(%d) = %d\n", key, *shared_mem);
		sleep(1);
	}

	shm_rm_key(key);
}


void
reading(unsigned int key)
{
	int *shared_mem, i;

	shared_mem = (int *)shm_get_buf(key, sizeof(int));

	for (i = 0; i < 5; i++) {
		printf("rec: shared_mem(%d) = %d\n", key, *shared_mem);
		usleep(500000); /* 0.5[sec]*/
	}
}


void
usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s {-w|-r} {-k num|-K num}\n", prog_name);
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
		opt = getopt(argc, argv, "wrk:K:h?");
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
			shm_rm_key(atoi(optarg));
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
