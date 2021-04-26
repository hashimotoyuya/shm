/* shm - shared memory library
 * Copyright(C) 2003-2004, Ozaki Lab., Utsunomiya Univ.
 * By Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>, 26 Nov. 2004
 * 2004-11-26: bugfix
 * 2004-10-08: 2nd relase
 * 2003-10-04: 1st release
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>

#include "shm.h"


void *
shm_get_buf(unsigned int key, unsigned int size)
{
	int shmid;
	unsigned char *p;
	unsigned int s, pid;

#ifdef DEBUG_SHM
	fprintf(stderr, "libshm.a(%s): shm_get_buf: size = %d", __FILE__, size);
	fprintf(stderr, "libshm.a(%s): shm_get_buf: key = %d", __FILE__, key);
#endif

	if (shm_read_info(key, &s, &pid, NULL) <= 0) {
		_shm_append_key(key, size, "-new-");
	} else if (s < size) {
		shm_rm_key(key);
		_shm_append_key(key, size, "-update-");
	}

	if ((shmid = shmget((key_t)key, size, IPC_CREAT | 0666)) < 0) {	
		perror("igl_shm_get_buf: shmget");
		return NULL;
	}

	if ((p = shmat(shmid, NULL, 0)) == (void *) -1) {
		perror("igl_shm_get_buf: shmat");
		return NULL;
	}

#ifdef DEBUG_SHM
	fprintf(stderr, "libshm.a(%s): shm_get_buf: p = %p\n", __FILE__, p);
#endif

	return (void *)p;
}


int
shm_rm_key(unsigned int key)
{
	int st;

	st = shmctl(shmget((key_t)key, 0, 0666), IPC_RMID, 0);
	_shm_delete_key(key);

	return st;
}
