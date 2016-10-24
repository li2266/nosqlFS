#include <stdio.h>
#include <errno.h>
#include <fuse.h>
#include <time.h>
#include <stdarg.h>

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
  sprinft(log_name, "nosqlFS_%d-%d-%d.log", p->tm_mon + 1, p->tm_mday, p->tm_year + 1900 );
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
  vfprint(nosqlFS_Data->logFile, format, ap);
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
  log_restat(func, retstat);
  if(retstat < min_ret){
    log_error(func);
    retstat = -errno;
  }
  return retstat;
}
