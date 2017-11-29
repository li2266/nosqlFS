#include "nosqlFS.h"

#include <bson.h>
#include <bcon.h>
#include <fuse.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/xattr.h>
#include <sys/types.h>

#include "log.h"
//#include "util.h"
#include "db_manager.h"
//#include "email_sender.h"
#include "helper.h"

#define XATTR_LIST_LENGTH 1024

/*

 */
static int nosqlFS_getattr(const char * path, struct stat * stbuf, struct fuse_file_info * fi) {
    /*
    int retstat = lstat(path, stbuf);
    if (retstat == 0 && S_ISREG(stbuf->st_mode)) {
        record_file_basic_info(path, stbuf);
    }
    return retstat;
    */
    (void) fi;
    int res;

    res = lstat(path, stbuf);
    if (res == -1)
        return -errno;

    if (res == 0 && S_ISREG(stbuf->st_mode)) {
        record_file_basic_info(path, stbuf);
    }
    log_msg("getattr: %s", path);
    return 0;
}

static int nosqlFS_access(const char * path, int mask) {
    int res;

    res = access(path, mask);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_readdir(const char * path, void * buf, fuse_fill_dir_t filler,
                           off_t offset, struct fuse_file_info * fi, enum fuse_readdir_flags flags) {

    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;
    (void) flags;

    dp = opendir(path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, 0))
            break;
    }

    closedir(dp);
    return 0;
}

static int nosqlFS_releasedir(const char * path, struct fuse_file_info * fi) {
    //log_msg("nosqlFS_releasedir(path = \"%s\", fi = 0x%08x)\n", path, fi);
    //do_bulk();

    //closedir((DIR *)(uintptr_t)fi->fh);
    return 0;
}

static int nosqlFS_readlink(const char * path, char * link, size_t size) {
    int res;

    res = readlink(path, link, size - 1);
    if (res == -1)
        return -errno;

    link[res] = '\0';
    return 0;
}

static int nosqlFS_mknod(const char * path, mode_t mode, dev_t rdev) {
    int res;

    /* On Linux this could just be 'mknod(path, mode, rdev)' but this
       is more portable */
    if (S_ISREG(mode)) {
        res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (res >= 0)
            res = close(res);
    } else if (S_ISFIFO(mode))
        res = mkfifo(path, mode);
    else
        res = mknod(path, mode, rdev);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_mkdir(const char * path, mode_t mode) {
    int res;

    res = mkdir(path, mode);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_unlink(const char * path) {
    int res;

    res = unlink(path);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_rmdir(const char * path) {
    int res;

    res = rmdir(path);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_symlink(const char * from, const char * to) {
    int res;

    res = symlink(from, to);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_rename(const char * from, const char * to, unsigned int flags) {
    int res;

    if (flags)
        return -EINVAL;

    res = rename(from, to);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_link(const char * from, const char * to) {
    int res;

    res = link(from, to);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_chmod(const char * path, mode_t mode, struct fuse_file_info * fi) {
    (void) fi;
    int res;

    res = chmod(path, mode);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_chown(const char * path, uid_t uid, gid_t gid, struct fuse_file_info * fi) {
    (void) fi;
    int res;

    res = lchown(path, uid, gid);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_truncate(const char * path, off_t size, struct fuse_file_info * fi) {

    int res;

    if (fi != NULL)
        res = ftruncate(fi->fh, size);
    else
        res = truncate(path, size);
    if (res == -1)
        return -errno;

    return 0;

}

#ifdef HAVE_UTIMENSAT
static int nosqlFS_utimens(const char * path, const struct timespec ts[2]) {
    (void) fi;
    int res;

    res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
    if (res == -1)
        return -errno;

    return 0;
}
#endif

static int nosqlFS_create(const char *path, mode_t mode,
              struct fuse_file_info *fi)
{
    int res;

    res = open(path, fi->flags, mode);
    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

static int nosqlFS_open(const char * path, struct fuse_file_info * fi) {
    int res;

    res = open(path, fi->flags);
    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

static int nosqlFS_read(const char * path, char * buf, size_t size, off_t offset,
                        struct fuse_file_info * fi) {
    int fd;
    int res;

    if(fi == NULL)
        fd = open(path, O_RDONLY);
    else
        fd = fi->fh;
    
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    if(fi == NULL)
        close(fd);
    return res;
}

static int nosqlFS_write(const char * path, const char * buf, size_t size, off_t offset,
                         struct fuse_file_info * fi) {
    int fd;
    int res;

    (void) fi;
    if(fi == NULL)
        fd = open(path, O_WRONLY);
    else
        fd = fi->fh;
    
    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    if(fi == NULL)
        close(fd);
    return res;
}

static int nosqlFS_statfs(const char * path, struct statvfs * stbuf) {
    int res;

    res = statvfs(path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int nosqlFS_flush(const char * path, struct fuse_file_info * fi) {
    //log_msg("nosqlFS_flush(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);
    return 0;
}

static int nosqlFS_release(const char * path, struct fuse_file_info * fi) {
    //log_msg("nosqlFS_release(path = \"%s\", fuse_file_info = 0x%08x)\n", path, fi);
    (void)path;
    close(fi->fh);
    return 0;
}

static int nosqlFS_setxattr(const char *path, const char *name, const char *value,
                            size_t size, int flags)
{
    //log_msg("nosqlFS_setxattr(path = \"%s\", name = \"%s\", value = \"%s\", size = %d, flags = %d)\n", path, name, value, size, flags);
    //int res = log_syscall("lsetxattr", lsetxattr(path, name, value, size, flags), 0);
    int res = lsetxattr(path, name, value, size, flags);
    //log_msg("parameters_value_after_call(path = \"%s\", name = \"%s\", value = \"%s\", size = %d, flags = %d)\n", path, name, value, size, flags);
    if (res == -1)
        return -errno;
    return 0;
}

static int nosqlFS_getxattr(const char *path, const char *name, char *value,
                            size_t size)
{
    //log_msg("nosqlFS_getxattr(path = \"%s\", name = \"%s\", value = \"%s\", size = %d)\n", path, name, value, size);
    //int retstat = log_syscall("lgetxattr", lgetxattr(path, name, value, size), 0);
    int retstat = lgetxattr(path, name, value, size);
    if (retstat >= 0) {
        //value[retstat] = '\0';
    }
    //log_msg("parameters_value_after_call(path = \"%s\", name = \"%s\", value = \"%s\", size = %d)\n", path, name, value, size);
    if (retstat == -1)
        return -errno;
    return retstat;
}

static int nosqlFS_listxattr(const char *path, char *list, size_t size)
{
    //log_msg("nosqlFS_listxattr(path = \"%s\", list = \"%s\", size = %d)\n", path, list, size);
    //int res = log_syscall("llistxattr", llistxattr(path, list, size), 0);
    int res = llistxattr(path, list, size);
    //log_msg("parameters_value_after_call(path = \"%s\", list = \"%s\", size = %d)\n", path, list, size);
    if (res == -1)
        return -errno;
    return res;
}

static int nosqlFS_removexattr(const char *path, const char *name)
{
    //log_msg("nosqlFS_removexattr(path = \"%s\", name = \"%s\")\n", path, name);
    //int res = log_syscall("lremovexattr", lremovexattr(path, name), 0);
    int res = lremovexattr(path, name);
    if (res == -1)
        return -errno;
    return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int nosqlFS_fallocate(const char *path, int mode,
            off_t offset, off_t length, struct fuse_file_info *fi)
{
    int fd;
    int res;

    (void) fi;

    if (mode)
        return -EOPNOTSUPP;

    if(fi == NULL)
        fd = open(path, O_WRONLY);
    else
        fd = fi->fh;
    
    if (fd == -1)
        return -errno;

    res = -posix_fallocate(fd, offset, length);

    if(fi == NULL)
        close(fd);
    return res;
}
#endif


static void *nosqlFS_init(struct fuse_conn_info *conn, struct fuse_config * cfg) {
    log_msg("nosqlFS_init()\n");
    log_conn(conn);
    (void) conn;
    log_fuse_context(fuse_get_context());

    int res = db_init();
    log_msg("mongodb init and returned %d\n", res);
    printf("mongodb init and returned %d\n", res);

    cfg->use_ino = 1;
    cfg->entry_timeout = 0;
    cfg->attr_timeout = 0;
    cfg->negative_timeout = 0;

    // nosqlFS_Data is a marco for (nosqlFS_state *)private_data
    return nosqlFS_Data;
}

static int nosqlFS_fsync(const char *path, int isdatasync,
             struct fuse_file_info *fi)
{
    /* Just a stub.  This method is optional and can safely be left
       unimplemented */

    (void) path;
    (void) isdatasync;
    (void) fi;
    return 0;
}

static void nosqlFS_usage() {
    fprintf(stderr, "usage: nosqlFS [options] rootDir mountPoint\n");
    abort();
}


static struct fuse_operations nosqlFS_oper = {
    .init = nosqlFS_init,
    .getattr = nosqlFS_getattr,
    .access = nosqlFS_access,
    .readlink = nosqlFS_readlink,
    //  .opendir = nosqlFS_opendir,
    .readdir = nosqlFS_readdir,
    .releasedir = nosqlFS_releasedir,
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
    .create = nosqlFS_create,
    .read = nosqlFS_read,
    .write = nosqlFS_write,
    .statfs = nosqlFS_statfs,
    //.flush = nosqlFS_flush,
    .release = nosqlFS_release,
    .fsync = nosqlFS_fsync,
#ifdef HAVE_POSIX_FALLOCATE
    .fallocate  = nosqlFS_fallocate,
#endif

#ifdef HAVE_SETXATTR
    .setxattr   = nosqlFS_setxattr,
    .getxattr   = nosqlFS_getxattr,
    .listxattr  = nosqlFS_listxattr,
    .removexattr    = nosqlFS_removexattr
#endif
};

int main(int argc, char * argv[]) {
    umask(0);
    int fuse_stat;
    // init the nosqlFS_Data which store the log file pointer
    struct nosqlFS_state * nosqlFS_init_data;
    printf("%s\n", "Creating nosqlFS_init_data");
    // init the nosqlFS_Data struct
    nosqlFS_init_data = malloc(sizeof(struct nosqlFS_state));
    if (nosqlFS_init_data == NULL) {
        perror("main calloc");
        abort();
    }
    printf("%s\n", "Creating finished");
    if (argc < 3) {
        perror("wrong parameters");
        nosqlFS_usage();
    }
    // deal with parameters
    //TODO: rootDir is useless now, between it's not important
    printf("%s%s\n", "p1", argv[1]);
    printf("%s%s\n", "p2", argv[2]);
    nosqlFS_init_data->rootdir = realpath(argv[argc - 1], NULL);
    printf("start creating log file\n");
    nosqlFS_init_data->logFile = log_open();
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    --argc;

    // start the main function of fuse
    fprintf(stderr, "start fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &nosqlFS_oper, nosqlFS_init_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}
