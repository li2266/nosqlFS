#include <fuse.h>
#include <stdio.h>
#include <errno.h>

static int nosqlFS_getattr(const char * path, struct stat * stbuf){
  int result;

  result = lstat(path, stbuf);
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

}
