#ifndef LOG_H
#define LOG_H
#include <stdio.h>

#define log_struct(st, field, format, typecast) log_msg( #field " = " #format "\n", typecast st->field)

FILE * log_open(void);
void log_msg(const char * format, ...);
void log_error(char * func);
void log_syscall(char * func, int retstat, int min_ret);
void log_retstat(char * func, int retstat);
void log_stat(struct stat * si);
