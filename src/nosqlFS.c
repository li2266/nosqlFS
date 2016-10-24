#include "nosqlFS.h"

#include <fuse.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"

int nosqlFS_getattr(const char * path, struct stat * stbuf){
  int result;
  log_msg("nosqlFS_getattr(char * path = %s, struct stat * stbuf = %08x)\n", path, stbuf);

  result = log_syscall("lstat", lstat(path, stbuf), 0);
  if(result == -1){
    return -errno;
  }
  return 0;
}

void *nosqlFS_init(struct fuse_conn_info *conn){
  log_msg("nosqlFS_init()\n");
  log_conn(conn);
  log_fuse_context(fuse_get_context());

  // nosqlFS_Data is a marco for (nosqlFS_state *)private_data
  return nosqlFS_Data;
}

struct fuse_operations nosqlFS_oper = {
  .getattr = nosqlFS_getattr,
  .init = nosqlFS_init
};

int main(int argc, char * argv[]){
  int fuse_stat;
  struct nosqlFS_state * nosqlFS_init_data;
  printf("%s\n", "Creating nosqlFS_init_data");
  nosqlFS_init_data = malloc(sizeof(struct nosqlFS_state));
  if(nosqlFS_init_data == NULL){
    perror("main calloc");
    abort();
  }
  printf("%s\n", "Creating finished");
  if(argc < 3){
    perror("wrong parameters");
    abort();
  }
  printf("%s%s\n", "p1", argv[1]);
  printf("%s%s\n", "p2", argv[2]);
  nosqlFS_init_data->rootdir = realpath(argv[argc - 1], NULL);
  printf("start creating log file\n");
  nosqlFS_init_data->logFile = log_open();
  argv[argc - 2] = argv[argc - 1];
  argv[argc - 1] = NULL;
  --argc;

  fprintf(stderr, "start fuse_main\n");
  fuse_stat = fuse_main(argc, argv, &nosqlFS_oper, nosqlFS_init_data);
  fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

  return fuse_stat;
}
