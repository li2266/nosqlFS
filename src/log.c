#include "nosqlFS.h"

#include <stdio.h>
#include <errno.h>
#include <fuse.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "log.h"

char time_buf[32];
/*
create a new log file or append log at the end of a exist file
*/
FILE * log_open(){
  // create  log file pointer
  FILE *logFile;
  // get current date
  time_t now = time(0);
  struct tm *p = gmtime(&now);
  // create log file name
  char log_name[30];
  sprintf(log_name, "../log/nosqlFS_%d-%d-%d.log", p->tm_mon + 1, p->tm_mday, p->tm_year + 1900 );
  // open the file and check if successful
  logFile = fopen(log_name, "a");
  if(logFile == NULL){
    perror("logFile");
    exit(EXIT_FAILURE);
  }
  setvbuf(logFile, NULL, _IOLBF, 0);
  return logFile;
}
/*
basic log function. write a line into logfile.
*/
void log_msg(const char *format, ...){
  va_list ap;
  va_start(ap, format);
  time_t timep = time(0);
  strftime(time_buf, sizeof(time_buf), "%m-%d-%Y %H:%M:%S ", localtime(&timep));
  fputs(time_buf, nosqlFS_Data->logFile);
  vfprintf(nosqlFS_Data->logFile, format, ap);
}
/*
log error and return -errno
*/
int log_error(char * func){
  log_msg("ERROR %s : %s\n", func, strerror(errno));
  return -errno;
}

void log_retstat(char *func, int retstat){
  int errsave = errno;
  log_msg("%s returned %d\n", func, retstat);
  errno = errsave;
}

int log_syscall(char * func, int retstat, int min_ret){
  log_retstat(func, retstat);
  if(retstat < min_ret){
    log_error(func);
    retstat = -errno;
  }
  return retstat;
}

void log_conn(struct fuse_conn_info * conn){
  log_msg("fuse_conn_info:\n");
  log_struct(conn, proto_major, %d, );
  log_struct(conn, proto_minor, %d, );
  log_struct(conn, async_read, %d, );
  log_struct(conn, max_write, %d, );
  log_struct(conn, max_readahead, %d, );
  log_struct(conn, capable, %08x, );
  log_struct(conn, want, %08x, );
  log_struct(conn, max_background, %d, );
  log_struct(conn, congestion_threshold, %d, );
}

void log_fuse_context(struct fuse_context * context){
  log_msg("fuse_context");
  log_struct(context, fuse, %08x, );
  log_struct(context, uid, %d, );
  log_struct(context, gid, %d, );
  log_struct(context, pid, %d, );
  log_struct(context, private_data, %08x, );
  log_struct(nosqlFS_Data, logFile, %08x, );
  log_struct(nosqlFS_Data, rootdir, %s, );
}

// struct fuse_file_info keeps information about files (surprise!).
// This dumps all the information in a struct fuse_file_info.  The struct
// definition, and comments, come from /usr/include/fuse/fuse_common.h
// Duplicated here for convenience.
void log_fi (struct fuse_file_info *fi){
  log_msg("fi:\n");

  /** Open flags.  Available in open() and release() */
  //	int flags;
	log_struct(fi, flags, 0x%08x, );

  /** In case of a write operation indicates if this was caused by a
        writepage */
  //	int writepage;
	log_struct(fi, writepage, %d, );

  /** Can be filled in by open, to use direct I/O on this file.
        Introduced in version 2.4 */
  //	unsigned int keep_cache : 1;
	log_struct(fi, direct_io, %d, );

  /** Can be filled in by open, to indicate, that cached file data
        need not be invalidated.  Introduced in version 2.4 */
  //	unsigned int flush : 1;
	log_struct(fi, keep_cache, %d, );

  /** Padding.  Do not use*/
  //	unsigned int padding : 29;

  /** File handle.  May be filled in by filesystem in open().
        Available in all other file operations */
  //	uint64_t fh;
	log_struct(fi, fh, 0x%016llx,  );

  /** Lock owner id.  Available in locking operations and flush */
  //  uint64_t lock_owner;
	log_struct(fi, lock_owner, 0x%016llx, );
}

// This dumps the info from a struct stat.  The struct is defined in
// <bits/stat.h>; this is indirectly included from <fcntl.h>
void log_stat(struct stat *si)
{
  log_msg("si:\n");

  //  dev_t     st_dev;     /* ID of device containing file */
	log_struct(si, st_dev, %lld, );

  //  ino_t     st_ino;     /* inode number */
	log_struct(si, st_ino, %lld, );

  //  mode_t    st_mode;    /* protection */
	log_struct(si, st_mode, 0%o, );

  //  nlink_t   st_nlink;   /* number of hard links */
	log_struct(si, st_nlink, %d, );

  //  uid_t     st_uid;     /* user ID of owner */
	log_struct(si, st_uid, %d, );

  //  gid_t     st_gid;     /* group ID of owner */
	log_struct(si, st_gid, %d, );

  //  dev_t     st_rdev;    /* device ID (if special file) */
	log_struct(si, st_rdev, %lld,  );

  //  off_t     st_size;    /* total size, in bytes */
	log_struct(si, st_size, %lld,  );

  //  blksize_t st_blksize; /* blocksize for filesystem I/O */
	log_struct(si, st_blksize, %ld,  );

  //  blkcnt_t  st_blocks;  /* number of blocks allocated */
	log_struct(si, st_blocks, %lld,  );

  //  time_t    st_atime;   /* time of last access */
	log_struct(si, st_atime, 0x%08lx, );

  //  time_t    st_mtime;   /* time of last modification */
	log_struct(si, st_mtime, 0x%08lx, );

  //  time_t    st_ctime;   /* time of last status change */
	log_struct(si, st_ctime, 0x%08lx, );

}
