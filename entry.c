/* shm - shared memory library
 * Copryright(C) 2003-2006, Ozaki Lab., Utsunomiya Univ.
 * By Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>, 26 Nov. 2004
 * 2010-02-04: bugfix: buf[strlen()] occurred buffer overflow in Ubuntu. 
 * 2008-11-08: bugfix: chmod() is fixed.
 * 2006-11-26: bugfix
 * 2006-07-14: modified: format of shm_show_info
 * 2006-07-14: bugfix
 * 2004-11-26: bugfix
 * 2004-10-08: 2nd release
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include "shm.h"

#define BUF_SIZE	256
#define WAIT_TIME	100000 /* us */
#define TMP_LIST	"/tmp/shm_tmp.list"
#define DEFAULT_PERM	( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH )

static int read_buf(int fd, char *buf);
static void read_line(char *buf, unsigned int *key, 
		      unsigned int *size, unsigned int *pid, char *p);
static inline int match_word(char *buf, char *word);
static int create_header(void);
static void cp(char *src, char *new);
static void lock(void);
static int _lock(void);
static void unlock(void);
static void mk_format(char *buf, 
		unsigned int key, unsigned int size, int pid, char *msg);


int
_shm_append_key(unsigned int key, unsigned int size, char *msg)
{
	FILE *fp;
	int fd;
	char buf[BUF_SIZE];

	fd = open(SHM_KEY_LIST, O_WRONLY);
	if (fd < 0) {
		fd = create_header();
		fd = open(SHM_KEY_LIST, O_APPEND | O_WRONLY);
	}

	lock();
	fp = fdopen(fd, "a");
	mk_format(buf, key, size, (int)getpid(), msg);
	fprintf(fp, "%s", buf);
	fclose(fp);
	close(fd);
	unlock();

	if (chmod(SHM_KEY_LIST, DEFAULT_PERM) != 0) {
		fprintf(stderr, "%s: _shm_append: chmod error\n", __FILE__);
	}

	return 0;
}


int
_shm_delete_key(unsigned int key)
{
	int fd0, fd1, k, n;
	char buf[BUF_SIZE];

	cp(SHM_KEY_LIST, TMP_LIST);

	fd0 = open(TMP_LIST, O_RDONLY);
	if (fd0 < 0) {
		fprintf(stderr, 
			"libshm.a(%s): _shm_delete_key: can't open file(%s)\n", 
			__FILE__, TMP_LIST);
		return fd0;
	}
	lock();

	fd1 = open(SHM_KEY_LIST, O_WRONLY | O_TRUNC, 0666);
	if (fd1 < 0) {
		fprintf(stderr, 
			"libshm.a(%s): _shm_delete_key: can't open file(%s)\n", 
			__FILE__, SHM_KEY_LIST);
		return fd1;
	}

	while ( (n = read_buf(fd0, buf)) > 0 ) {
#ifdef DEBUG_ENTRY
		fprintf(stderr, 
			"libshm.a(%s): _shm_delete_key: buf[] = %s(%d)", 
			__FILE__, buf, n);
#endif
		if (buf[0] == '#') {
			buf[n] = '\n';
			write(fd1, buf, n + 1);

//getchar();
		} else {
			sscanf(buf, "%d", &k);

			if (k == key) {
#ifdef DEBUG_ENTRY
				fprintf(stderr, ": remove");
#endif

			} else {
				if (buf[0] != '\0' && buf[0] != '\n') {
					buf[n] = '\n';
					write(fd1, buf, n + 1);

				}
			}
		} 
#ifdef DEBUG_ENTRY
		fprintf(stderr, "\n");
#endif
	}

	close(fd1);
	close(fd0);
	unlink(TMP_LIST);

	unlock();

	return 0;
}


int
shm_write_info(unsigned int key, char *info)
{
	int fd0, fd1, k, n;
	int size, pid; 
	char buf[BUF_SIZE];

	fd0 = open(SHM_KEY_LIST, O_RDONLY);
	if (fd0 < 0) {
		fprintf(stderr, 
			"libshm.a(%s): shm_write_info: can't open file(%s)\n", 
			__FILE__, TMP_LIST);
		return fd0;
	}
	lock();

	fd1 = open(TMP_LIST, O_WRONLY | O_CREAT, 0666);
	if (fd1 < 0) {
		fprintf(stderr, 
			"libshm.a(%s): shm_write_info: can't open file(%s)\n", 
			__FILE__, SHM_KEY_LIST);
		return fd1;
	}

	while ( (n = read_buf(fd0, buf)) > 0 ) {
#ifdef DEBUG_ENTRY
		fprintf(stderr, 
		        "libshm.a(%s): shm_write_info: buf[%d] = %s\n", 
			__FILE__, n, buf);
#endif

		if (buf[0] == '#') {
			buf[n] = '\n';
			write(fd1, buf, n + 1);
		} else {
			sscanf(buf, "%d %d %d", &k, &size, &pid);
			if (k == key) {
				mk_format(buf, k, size, pid, info);
				write(fd1, buf, strlen(buf));
#ifdef DEBUG_ENTRY
				fprintf(stderr, 
		 		       "libshm.a(%s): shm_write_info: overwrite: %s\n", 
					__FILE__, buf);
				fprintf(stderr, 
		 		       "libshm.a(%s): shm_write_info: buf size = %d\n", __FILE__, strlen(buf));
#endif
			} else {
				if (buf[0] != '\0' && buf[0] != '\n') {
					buf[n] = '\n';
					write(fd1, buf, n + 1);
				}
			}
		}
	}

	close(fd1);
	close(fd0);

	unlink(SHM_KEY_LIST);
	chmod(TMP_LIST, DEFAULT_PERM);
	cp(TMP_LIST, SHM_KEY_LIST);
	unlink(TMP_LIST);
	
	unlock();

	if (chmod(SHM_KEY_LIST, DEFAULT_PERM) != 0) {
		fprintf(stderr, "%s: shm_write_info: chmod error\n", __FILE__);
	}

	return 0;
}


int
shm_read_info(unsigned int key, unsigned int *size, unsigned int *pid, 
	      char *info)
{
	int fd;
	unsigned int k, n;
	char buf[BUF_SIZE];

	fd = open(SHM_KEY_LIST, O_RDONLY);
	if (fd < 0) {
		return -1;
	}
	lock();

	*size = 0;
	while ( (n = read_buf(fd, buf) ) > 0) {

#ifdef DEBUG_ENTRY
		fprintf(stderr, 
			"libshm.a(%s): shm_read_info: buf[] = %s(%d)\n",
			__FILE__, buf, n);
#endif
		if (buf[0] != '#') {
			sscanf(buf, "%d", &k);
			if (k == key) {
				read_line(buf, &k, size, pid, info);
#ifdef DEBUG_ENTRY
				fprintf(stderr, 
				        "libshm.a(%s): shm_read_info: found: key = %d\n", 
					__FILE__, key);
				fprintf(stderr, 
				        "libshm.a(%s): shm_read_info: found: size = %d\n", 
					__FILE__, *size);
				fprintf(stderr, 
				        "libshm.a(%s): shm_read_info: found: pid = %d\n", 
					__FILE__, *pid);
#endif
				break;
			}
		}
	}

	close(fd);
	unlock();
#ifdef DEBUG_ENTRY
	fprintf(stderr, "libshm.a(%s): shm_read_info: return: *size  = %d\n", 
		__FILE__, *size);
#endif

	return *size;
}


int
shm_create_header(void)
{
	return create_header();
}


static int
read_buf(int fd, char *buf)
{
	int i, n;

	for (i = 0; i < BUF_SIZE; i++) {
		n = read(fd, &buf[i], 1);
		if (n <= 0) {
			buf[i] = '\0';
			return 0;
		} else if (buf[i] == '\0' || buf[i] == '\n') {
			buf[i] = '\0';
			return i;
		}
	}

	return -1;
}


static void
read_line(char *buf, unsigned int *key, unsigned int *size, unsigned int *pid, 
	  char *p)
{
	int i;

#ifdef DEBUG_ENTRY
	fprintf(stderr, "libshm.a(%s): read_line: buf = %s\n", __FILE__, buf);
#endif

	if (p == NULL) {
		sscanf(buf, "%d %d %d", key, size, pid);
		return;
	} else {
		sscanf(buf, "%d %d %d %s", key, size, pid, p);
	}

	while (match_word(buf, p) == 1) {
		buf++;
	}

	for (i = 0; buf[i] != '\0'; i++) {
		p[i] = buf[i];
	}
	p[i] = '\0';

#ifdef DEBUG_ENTRY
	fprintf(stderr, "libshm.a(%s): read_line: key = %d\n", __FILE__, *key);
	fprintf(stderr, "libshm.a(%s): read_line: size = %d\n", __FILE__, *size);
	fprintf(stderr, "libshm.a(%s): read_line: pid = %d\n", __FILE__, *pid);
	fprintf(stderr, "libshm.a(%s): read_line: *p = %s(%d)\n", __FILE__, p, i);
	fprintf(stderr, "libshm.a(%s): read_line: *p = ", __FILE__);
	{
		int j;
		for (j = 0; j <= i; j++) {
			if (p[j] > ' ' && p [j] < 0x7f) {
				fprintf(stderr, "%c ", p[j]);
			} else {
				fprintf(stderr, "[%02x] ", p[j]);
			}
		}
		fprintf(stderr, "\n");
	}
#endif
}


static inline int
match_word(char *buf, char *word)
{
	int i;

	for (i = 0; word[i] != '\0'; i++) {
		if (buf[i] != word[i]) {
			return 1;
		}
	}

	return 0;
}


static int
create_header(void)
{
	FILE *fp;

	fp = fopen(SHM_KEY_LIST, "w");
	if (fp < 0) {
		fprintf(stderr, 
			"libshm.a(%s): create_header: can't creat file(%s)\n", 
			__FILE__, SHM_KEY_LIST);
		return 1;
	}

	fprintf(fp, "# sharde memory entry list by shm library\n");
	fprintf(fp, "# Copyright(C) 2004-2006, Ozaki Lab., Utsunomiya Univ.\n");
	fprintf(fp, "# By Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>\n");
	fprintf(fp, "# ---\n");
	fprintf(fp, "# key\t\tsize(Bytes)\tPID(Creator)\tInfo.\n");

	fclose(fp);
	if (chmod(SHM_KEY_LIST, DEFAULT_PERM) != 0) {
		fprintf(stderr, "%s: create_header: chmod error\n", __FILE__);
	}

	return 0;
}


static void
cp(char *src, char *new)
{
	int fd0, fd1, n;
	char buf[BUF_SIZE];

	fd0 = open(src, O_RDONLY); 
	if (fd0 < 0) {
		fprintf(stderr, 
			"libshm.a(%s): cp: can't open file(%s)\n", 
			__FILE__, src);
		return;
	}

	fd1 = open(new, O_WRONLY | O_CREAT, 0666);
	if (fd1 < 0) {
		fprintf(stderr, 
			"libshm.a(%s): cp: can't open file(%s)\n", 
			__FILE__, new);
		return;
	}


	while ((n = read(fd0, buf, BUF_SIZE)) > 0) {
		write(fd1, buf, n);
	}

	close(fd0);
	close(fd1);
}


static void
lock(void)
{
	while (_lock()) {
		usleep(WAIT_TIME);
	}
}


static int
_lock(void)
{
	static int c;
	int fd;
	char buf[BUF_SIZE];

	sprintf(buf, "%s.lock", SHM_KEY_LIST);
	fd = open(buf, O_RDWR | O_CREAT | O_EXCL, DEFAULT_PERM);
	if (fd < 0) {
		if (c > 20) {
			fprintf(stderr, 
			"libshm.a(%s): lock: can't create lockfile(%s)\n", 
			__FILE__, buf);
			c = 0;
		} else {
			c++;
		}
		return 1;
	}
	close(fd);

	return 0;
}


static void
unlock(void)
{
	char buf[BUF_SIZE];

	sprintf(buf, "%s.lock", SHM_KEY_LIST);
	unlink(buf);
}


static void
mk_format(char *buf, unsigned int key, unsigned int size, int pid, char *msg)
{
	char *p;

	for (p = msg; *p != '\0'; p++) {
		if ('\0' < *p && *p < ' ' ) {
			fprintf(stderr, "%s: mk_format: warnning info(%s)\n",
				__FILE__, msg);
			*p = '\0';
		}
	}

	p = getenv("USER");

	if (p == NULL) {
		if (key >= 10000000) {
			sprintf(buf, "%d\t%d\t\t%d\t\t%s\n", 
				key, size, pid, msg);
		} else {
			sprintf(buf, "%d\t\t%d\t\t%d\t\t%s\n", 
				key, size, pid, msg);
		}
	} else {
		if (key >= 10000000) {
			sprintf(buf, "%d\t%d\t\t%d\t\t%s: %s\n", 
				key, size, pid, p, msg);
		} else	{	
			sprintf(buf, "%d\t\t%d\t\t%d\t\t%s: %s\n", 
				key, size, pid, p, msg);
		}
	}
}
