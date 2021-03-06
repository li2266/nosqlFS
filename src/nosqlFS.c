#include "nosqlFS.h"

#include <fuse.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include "log.h"

/*

*/
static int nosqlFS_getattr(const char * path, struct stat * stbuf){
  log_msg("nosqlFS_getattr(char * path = %s, struct stat * stbuf = %08x)\n", path, stbuf);

  return log_syscall("lstat", lstat(path, stbuf), 0);
}

static int nosqlFS_access(const char * path, int mask){
  if(access(path, mask) < 0){
    return -errno;
  }
  return 0;
}

// static int nosqlFS_opendir(const char * path, struct fuse_file_info *fi){
//   DIR * dp;
//   int retstat = 0;
//
//   log_msg("nosqlFS_opendir(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);
//   // we need the return value of opendir, so we couldn't use syscall
//   dp = opendir(path);
//   if(dp == NULL){
//     retstat = log_error("nosqlFS_opendir");
//   }
//   fi->fh = (intptr_t)dp;
//   return retstat;
// }

static int nosqlFS_readdir(const char * path, void * buf, fuse_fill_dir_t filler,
  off_t offset, struct fuse_file_info * fi, enum fuse_readdir_flags flags){

  DIR * dp;
  struct dirent *de;
  int retstat = 0;

  (void)offset;
  (void)fi;
  (void)flags;

  log_msg("nosqlFS_opendir(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);
  dp = opendir(path);
  if(dp == NULL){
    retstat = log_error("nosqlFS_opendir");
    return retstat;
  }

  log_msg("nosqlFS_readdir(path = \"%s\", buf = 0x%08x, fuse_fill_dir_t = 0x%08x, offset = %lld, fi = 0x%08x)\n",
    path, buf, filler, offset, fi);

  //dp = (DIR *)(uintptr_t)fi->fh;
  while((de = readdir(dp)) != NULL){
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;

    log_msg("calling filler with name %s\n", de->d_name);

    if(filler(buf, de->d_name, &st, 0, 0)){
      break;
    }
  }
  closedir(dp);
  return retstat;
}

static int nosqlFS_releasedir(const char * path, struct fuse_file_info * fi){
  log_msg("nosqlFS_releasedir(path = \"%s\", fi = 0x%08x)\n", path, fi);

  //closedir((DIR *)(uintptr_t)fi->fh);
  return 0;
}

static int nosqlFS_readlink(const char * path, char * link, size_t size){
  int retstat;

  log_msg("nosqlFS_readlink(path = \"%s\", link = \"S\", size = %d)\n", path, link, path);
  retstat = log_syscall("readlink", readlink(path, link, size - 1), 0);
  if(retstat > 0){
    link[retstat] = '\0';
    return 0;
  }
  return retstat;
}

static int nosqlFS_mknod(const char * path, mode_t mode, dev_t rdev){
  int retstat;

  log_msg("nosqlFS_mknod(path = \"%s\", mode = 0%3o, dev = %lld)\n", path, mode, rdev);

  if(S_ISREG(mode)){
    retstat = log_syscall("open", open(path, O_CREAT | O_EXCL | O_WRONLY, mode), 0);
    if(retstat >= 0){
      retstat = log_syscall("close", close(retstat), 0);
    }
  }else if(S_ISFIFO(mode)){
    retstat = log_syscall("mkfifio", mkfifo(path, mode), 0);
  }else{
    retstat = log_syscall("mknod", mknod(path, mode, rdev), 0);
  }
  // open will return file descriptor, so if restat != -1, return 0;
  if(retstat == -1){
    return -errno;
  }
  return 0;
}

static int nosqlFS_mkdir(const char * path, mode_t mode){
  log_msg("nosqlFS_mkdir(path = \"%s\", mode = 0%3o)\n", path, mode);

  return log_syscall("mkdir", mkdir(path, mode), 0);
}

static int nosqlFS_unlink(const char * path){
  log_msg("nosqlFS_unlink(path = \"%s\")\n", path);

  return log_syscall("unlink", unlink(path), 0);
}

static int nosqlFS_rmdir(const char * path){
  log_msg("nosqlFS_rmdir(path = \"%s\")\n", path);

  return log_syscall("rmdir", rmdir(path), 0);
}

static int nosqlFS_symlink(const char * from, const char * to){
  log_msg("nosqlFS_symlink(from = \"%s\", to = \"%s\")\n", from, to);

  return log_syscall("symlink", symlink(from, to), 0);
}

static int nosqlFS_rename(const char * from, const char * to, unsigned int flags){
  log_msg("nosqlFS_rename(from = \"%s\", to = \"%s\", flags = %u)\n", from, to, flags);

  return log_syscall("rename", rename(from, to), 0);
}

static int nosqlFS_link(const char * from, const char * to){
  log_msg("nosqlFS_link(from = \"%s\", to = \"%s\")\n", from, to);

  return log_syscall("link", link(from, to), 0);
}

static int nosqlFS_chmod(const char * path, mode_t mode){
  log_msg("nosqlFS_chmod(path = \"%s\", mode = 0%3o)\n", path, mode);

  return log_syscall("chmod", chmod(path, mode), 0);
}

static int nosqlFS_chown(const char * path, uid_t uid, gid_t gid){
  log_msg("nosqlFS_chown(path = \"%s\", uid = %d, gid = %d)\n", path, uid, gid);

  return log_syscall("lchown", lchown(path, uid, gid), 0);
}

static int nosqlFS_truncate(const char * path, off_t size){
  log_msg("nosqlFS_truncate(path = \"%s\", size = %lld)\n", path, size);

  return log_syscall("truncate", truncate(path, size), 0);
}

#ifdef HAVE_UTIMENSAT
static int nosqlFS_utimens(const char * path, const struct timespec ts[2]){
  log_msg("nosqlFS_utimens(path = \"%s\", timespec)\n", path);

  return log_utimens("utimens", utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW), 0);
}
#endif

static int nosqlFS_open(const char * path, struct fuse_file_info * fi){
  int retstat;

  log_msg("nosqlFS_open(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);

  retstat = log_syscall("open", open(path, fi->flags), 0);
  //fi->fh = fd;
  // if there is an error, error will be logged by log_error in log_syscall,
  // however, the error value will still be store in fi_fh used as file handler
  //TODO: record information of fi.
  log_fi(fi);
  if(retstat == -1){
    return -errno;
  }
  close(retstat);
  return 0;
}

static int nosqlFS_read(const char * path, char * buf, size_t size, off_t offset,
  struct fuse_file_info * fi){
  int fd;
  int retstat;

  (void)fi;
  fd = log_syscall("open", open(path, O_RDONLY), 0);
  if(fd == -1){
    return -errno;
  }
  log_msg("nosqlFS_read(path = \"%s\", buf = 0x%08x, size = %d, offset = %lld, fuse_file_info = 0x%08x)\n", path, buf, size, offset, fi);

  retstat = log_syscall("pread", pread(fd, buf, size, offset), 0);
  close(fd);
  return retstat;
}

static int nosqlFS_write(const char * path, const char * buf, size_t size, off_t offset,
  struct fuse_file_info * fi){
  int fd;
  int retstat;

  (void)fi;
  log_msg("nosqlFS_write(path = \"%s\", buf = 0x%08x, size = %d, offset = %lld, fuse_file_info = 0x%08x)\n", path, buf, size, offset, fi);
  fd = log_syscall("open", open(path, O_WRONLY), 0);
  if(fd == -1){
    return -errno;
  }

  retstat = log_syscall("pwrite", pwrite(fd, buf, size, offset), 0);
  close(fd);
  return retstat;
}

static int nosqlFS_statfs(const char * path, struct statvfs * stbuf){
  log_msg("nosqlFS_statfs(path = \"%s\", statvfs = 0x%08x)\n", path, stbuf);

  return log_syscall("statvfs", statvfs(path, stbuf), 0);
}

static int nosqlFS_flush(const char * path, struct fuse_file_info * fi){
  log_msg("nosqlFS_flush(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);
  return 0;
}

static int nosqlFS_release(const char * path, struct fuse_file_info * fi){
  log_msg("nosqlFS_release(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);
  (void)path;
  (void)fi;
  return 0;
}

static void *nosqlFS_init(struct fuse_conn_info *conn){
  log_msg("nosqlFS_init()\n");
  log_conn(conn);
  log_fuse_context(fuse_get_context());

  // nosqlFS_Data is a marco for (nosqlFS_state *)private_data
  return nosqlFS_Data;
}

static void nosqlFS_usage(){
  fprintf(stderr, "usage: nosqlFS [options] rootDir mountPoint\n");
  abort();
}

static struct fuse_operations nosqlFS_oper = {
  .getattr = nosqlFS_getattr,
  .access = nosqlFS_access,
  .readlink = nosqlFS_readlink,
//  .opendir = nosqlFS_opendir,
  .readdir = nosqlFS_readdir,
//  .releasedir = nosqlFS_releasedir,
  .mknod = nosqlFS_mknod,
  .mkdir = nosqlFS_mkdir,
  .symlink = nosqlFS_symlink,
  .unlink = nosqlFS_unlink,
  .rmdir = nosqlFS_rmdir,
  .rename = nosqlFS_rename,
  .link = nosqlFS_link,
  .chmod = nosqlFS_chmod,
  .chown = nosqlFS_chown,
  .truncate = nosqlFS_truncate,
#ifdef HAVE_UTIMENSAT
  .utimens = nosqlFS_utimens,
#endif
  .open = nosqlFS_open,
  .read = nosqlFS_read,
  .write = nosqlFS_write,
  .statfs = nosqlFS_statfs,
//  .flush = nosqlFS_flush,
  .release = nosqlFS_release,
//  .fsync = nosqlFS_fsync,
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
    nosqlFS_usage();
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
