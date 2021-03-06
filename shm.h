/* shm - shared memory library
 * Copyright(C) 2003-2004, Ozaki Lab., Utsunomiya Univ.
 * By Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>
 * 2005-10-04: 2nd release, some functions are added
 * 2004-11-05: 1nt release
 */
#ifndef __SHM__
#define __SHM__

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_VERSION	"20100204"

#define SHM_KEY_LIST	"/tmp/shm_info.txt"


/* shm.c */
void *		shm_get_buf(unsigned int key, unsigned int size);
int 		shm_rm_key(unsigned int key);

/* entry.c */
int		_shm_append_key(unsigned int key, unsigned int size, char *msg);
int		_shm_delete_key(unsigned int key);
int		shm_write_info(unsigned int key, char *info);
int		shm_read_info(unsigned int key, 
			unsigned int *size, unsigned int *pid, char *info);
int		_shm_create_haader(void);

#endif /* __SHM__ */
