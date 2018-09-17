// vim: noet:ts=4:sts=4:sw=4:et
#define FUSE_USE_VERSION 26
#define _GNU_SOURCE
 
#include <assert.h>
#include <endian.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <iconv.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "vfat_boot.h"
#include "vfat_cluster.h"
#include "vfat_directory.h"
#include "debugfs.h"
#include "vfat_cluster.h"
#include "util.h"


#define DEBUG_PRINT(...) printf(__VA_ARGS__)

iconv_t iconv_utf16;
char* DEBUGFS_PATH = "/.debug";



// Get file attributes
int vfat_fuse_getattr(const char *path, struct stat *st)
{
    if (strncmp(path, DEBUGFS_PATH, strlen(DEBUGFS_PATH)) == 0) {
        // This is handled by debug virtual filesystem
        return debugfs_fuse_getattr(path + strlen(DEBUGFS_PATH), st);
    }
    else if (strcmp(path, "/") == 0) {
        st->st_dev = 0;
        st->st_ino = 0;
        st->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | S_IFDIR;
        st->st_nlink = 1;
        st->st_uid = vfat_info.mount_uid;
        st->st_gid = vfat_info.mount_gid;
        st->st_rdev = 0;
        st->st_size = 0;
        st->st_blksize = 0;
        st->st_blocks = 1;
        return 0;
    }
    else {
        return vfat_resolve(path, st);
    }
}

// Extended attributes useful for debugging
int vfat_fuse_getxattr(const char *path, const char* name, char* buf, size_t size)
{

    struct stat st;
    int ret = vfat_resolve(path, &st);
    if (ret != 0) return ret;
    if (strcmp(name, "debug.cluster") != 0) return -ENODATA;

    if (buf == NULL) {
        ret = snprintf(NULL, 0, "%u", (unsigned int) st.st_ino);
        if (ret < 0) err(1, "WTF?");
        return ret + 1;
    } else {
        ret = snprintf(buf, size, "%u", (unsigned int) st.st_ino);
        if (ret >= size) return -ERANGE;
        return ret;
    }
}

int vfat_fuse_readdir(
        const char *path, void *callback_data,
        fuse_fill_dir_t callback, off_t unused_offs, struct fuse_file_info *unused_fi)
{

    if (strncmp(path, DEBUGFS_PATH, strlen(DEBUGFS_PATH)) == 0) {
        // This is handled by debug virtual filesystem
        return debugfs_fuse_readdir(path + strlen(DEBUGFS_PATH), callback_data, callback, unused_offs, unused_fi);
    }

    struct stat stat;

    int hasError = vfat_resolve(path,&stat);

    if(hasError == 0){
        vfat_readdir(stat.st_ino,callback,callback_data);
    }

    return hasError;
}


int vfat_fuse_read(
        const char *path, char *buf, size_t size, off_t offs,
        struct fuse_file_info *unused)
{

    if (strncmp(path, DEBUGFS_PATH, strlen(DEBUGFS_PATH)) == 0) {
        // This is handled by debug virtual filesystem
        return debugfs_fuse_read(path + strlen(DEBUGFS_PATH), buf, size, offs, unused);
    }

    struct stat stat;

    int hasError = vfat_resolve(path,&stat);

    if(hasError){
        return errno;
    }

    int cluster_chain_length = cluster_chain_size(stat.st_ino);
    unsigned char * clusters_content = calloc(cluster_chain_length, vfat_info.cluster_size);

    if(clusters_content == NULL){
        return errno;
    }

    int success_read = load_cluster_chain(clusters_content,stat.st_ino);


    if(success_read != cluster_chain_length){
        free(clusters_content);
        return errno;
    }

    size_t len_to_read = stat.st_size-offs;

    if (len_to_read < 0) return 0;

    if(size<len_to_read){
        len_to_read = size;
    }

    memcpy(buf, clusters_content+offs,len_to_read);

    free(clusters_content);

    return len_to_read;
}


////////////// No need to modify anything below this point
int
vfat_opt_args(void *data, const char *arg, int key, struct fuse_args *oargs)
{
    if (key == FUSE_OPT_KEY_NONOPT && !vfat_info.dev) {
        vfat_info.dev = strdup(arg);
        return (0);
    }
    return (1);
}

struct fuse_operations vfat_available_ops = {
    .getattr = vfat_fuse_getattr,
    .getxattr = vfat_fuse_getxattr,
    .readdir = vfat_fuse_readdir,
    .read = vfat_fuse_read,
};


int main(int argc, char **argv)
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    fuse_opt_parse(&args, NULL, NULL, vfat_opt_args);

    if (!vfat_info.dev)
        errx(1, "missing file system parameter");

    vfat_init(vfat_info.dev);

    int a = (fuse_main(args.argc, args.argv, &vfat_available_ops, NULL));

    unmap(vfat_info.fat,vfat_info.fat_size);

    return a;
}
