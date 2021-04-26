/* shm_show_info - a tool for shared memory library (shm)
 * Copyright(C) 2004, Ozaki Lab., Utsunomiya Univ.
 * By Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>, 2004-10-08
 * 2004-11-17: addition of the option -R for deleting all shm. 
 * 2004-10-08: 1st release
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shm.h"
#define BUF_SIZE	1024

static char *prog_name;


void
rm_all(void)
{
	FILE *fp;
	int key;
	char buf[BUF_SIZE];

	fp = fopen(SHM_KEY_LIST, "r");
	if (fp == 0) {
		fprintf(stderr, "%s: can't open %s\n", prog_name, SHM_KEY_LIST);
		exit(1);
	}

	while (fgets(buf, BUF_SIZE, fp) != NULL) {
		if (buf[0] != '#') {
			sscanf(buf, "%d", &key);
			shm_rm_key(key);
		}
	}
	fclose(fp);
	exit(0);
}


void
show_info(int key)
{
	unsigned int size, pid;
	char info[BUF_SIZE];

	if (shm_read_info(key, &size, &pid, info) > 0) {
		printf("key = %d\n", key);
		printf("size = %d\n", size);
		printf("creator PID = %d\n", pid);
		printf("info = %s\n\n", info);
	} else {
		printf("key = %d:\tno entry\n", key);
	}
}


void
show_all(void)
{
	int fd;
	char buf;

	fd = open(SHM_KEY_LIST, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s: can't open %s\n", prog_name, SHM_KEY_LIST);
		exit(1);
	}

	while (read(fd, &buf, 1) > 0) {
		printf("%c", buf);
	}
	close(fd);
	exit(0);
}


void
usage(void)
{
	fprintf(stderr, "shm version: %s\n", SHM_VERSION);
	fprintf(stderr, "Usage: %s [options] [key num...]\n", prog_name);
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t-a \tshow all information\n");
	fprintf(stderr, "\t-r \tremove shared memory \n");
	fprintf(stderr, "\t-R \tremove all shared memory\n");
	fprintf(stderr, "\t-h, ?\tshow this message\n");
	exit(1);
}


int
main(int argc, char **argv)
{
	int rm_flag = 0;
	int opt, i, key;

	prog_name = argv[0];

	do {
		opt = getopt(argc, argv, "arRh?");
		switch (opt) {
		case 'a':
			show_all();
			break;
		case 'r':
			rm_flag = 1;
			break;
		case 'R':
			rm_all();
			break;
		case 'h':
		case '?':
			usage();
		}
	} while (opt > 0);

	if (argc == 1) {
		show_all();
		exit(0);
	}

	for (i = 0; i < argc; i++) {
		key = atoi(argv[i]); 
		if (key > 0) {
			if (rm_flag == 0) {
				show_info(key);
			} else {
				shm_rm_key(key);
			}
		}
	}

	return 0;
}
