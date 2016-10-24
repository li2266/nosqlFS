#include <fuse.h>
#include <stdio.h>
#include <errno.h>

#include "log.h"

static int nosqlFS_getattr(const char * path, struct stat * stbuf){
  int result;
  log_msg("nosqlFS_getattr(char * path = %s, struct stat * stbuf = %08x)", path, stbuf);

  result = log_syscall("lstat", lstat(path, stbuf), 0);
  if(result == -1){
    return -errno;
  }
  return 0;
}

struct fuse_operations nosqlFS_oper = {
  .getattr = nosqlFS_getattr,
  .init = nosqlFS_init
}

int main(int argc, char * argv){
  int fuse_stat;
  struct nosqlFS_state * nosqlFS_data;
  nosqlFS_data = malloc(sizeof(struct nosqlFS_data));
  if(nosqlFS_data == NULL){
    perror("main calloc");
    abort();
  }
  if(argc < 3){
    perror("wrong parameters");
    abort();
  }
  nosqlFS_Data->rootdir = realpath(argv[argc - 1], NULL);
  nosqlFS_Data->logfile = log_open();
  fuse_stat = fuse_main(argc, argv, &nosqlFS_oper, nosqlFS_Data);

  return fuse_stat;
}
