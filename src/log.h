#ifndef LOG_H
#define LOG_H

#define FUSE_USE_VERSION 30

#include <stdio.h>

#define log_struct(st, field, format, typecast) log_msg( #field " = " #format "\n", typecast st->field)

FILE * log_open(void);
void log_msg(const char * format, ...);
int log_error(char * func);
int log_syscall(char * func, int retstat, int min_ret);
void log_retstat(char * func, int retstat);
void log_stat(struct stat * si);
void log_conn(struct fuse_conn_info * conn);
void log_fuse_context(struct fuse_context * context);
void log_fi(struct fuse_file_info *fi);
void log_stat(struct stat *si);

#endif
