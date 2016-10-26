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
log error and return errno
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
