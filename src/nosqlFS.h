#ifndef NOSQLFS_H
#define NOSQLFS_H

#define FUSE_USE_VERSION 30
#define _XOPEN_SOURCE 700

#include <limits.h>
#include <stdio.h>

struct nosqlFS_state {
        FILE *logFile;
        char *rootdir;
};

#define nosqlFS_Data ((struct nosqlFS_state *) fuse_get_context()->private_data)

#endif
