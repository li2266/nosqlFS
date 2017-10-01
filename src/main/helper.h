#ifndef HELPER_H
#define HELPER_H

#include <sys/stat.h>

void record_file_basic_info(const char * path, struct stat * stbuf);

#endif