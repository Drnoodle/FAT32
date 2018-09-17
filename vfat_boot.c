




//_________________________________________________________________  PRIVATE


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
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>

#include "vfat_boot.h"
#include "vfat_cluster.h"
#include "vfat_directory.h"
#include "util.h"
#include "debugfs.h"


iconv_t iconv_utf16;



//_________________________________________________________________  PUBLIC FUNCTIONS


void vfat_init(const char *dev)
{
    fprintf(stderr,"try");

    struct fat_boot_header s;

    iconv_utf16 = iconv_open("utf-8", "utf-16"); // from utf-16 to utf-8
    // These are useful so that we can setup correct permissions in the mounted directories
    vfat_info.mount_uid = getuid();
    vfat_info.mount_gid = getgid();

    // Use mount time as mtime and ctime for the filesystem root entry (e.g. "/")
    vfat_info.mount_time = time(NULL);

    vfat_info.fd = open(dev, O_RDONLY);
    if (vfat_info.fd < 0)
        err(1, "open(%s)", dev);
    if (pread(vfat_info.fd, &s, sizeof(s), 0) != sizeof(s))
        err(1, "read super block");

    // control if the boot loaded is valid
    int is_a_valid_FAT32_boot =
            s.root_max_entries == 0
            && s.total_sectors_small == 0
            && s.sectors_per_fat_small == 0;

    int i = 0;
    while(i<12){
        is_a_valid_FAT32_boot = is_a_valid_FAT32_boot & (s.reserved2[i] == 0);
        i++;
    }

    // TODO : signature ?

    if(!is_a_valid_FAT32_boot){
        errx(1, "FAT32 boot seems to be corrupted");
    }

    vfat_info.root_cluster = s.root_cluster & 0x0FFFFFFF; // mask 4 upper bits
    vfat_info.cluster_begin_offset = (s.reserved_sectors+s.fat_count*s.sectors_per_fat)*s.bytes_per_sector;
    vfat_info.direntry_per_cluster = s.sectors_per_cluster*s.bytes_per_sector/sizeof(struct fat32_direntry);
    vfat_info.bytes_per_sector = s.bytes_per_sector;
    vfat_info.sectors_per_cluster = s.sectors_per_cluster;
    vfat_info.reserved_sectors = s.reserved_sectors;
    vfat_info.sectors_per_fat = s.sectors_per_fat;
    vfat_info.cluster_size = s.sectors_per_cluster*s.bytes_per_sector;

    vfat_info.fat_begin_offset = s.reserved_sectors*s.bytes_per_sector;

    vfat_info.fat_size =  s.fat_count*s.sectors_per_fat*s.bytes_per_sector;
    vfat_info.fat_entries = vfat_info.fat_size/8;


    vfat_info.fat = mmap_file(vfat_info.fd, vfat_info.fat_begin_offset, vfat_info.fat_size);

    if(vfat_info.fat == NULL){
        errx(1, "error to map the file in mem");
    }

    vfat_info.root_inode.st_ino = le32toh(s.root_cluster);
    vfat_info.root_inode.st_mode = 0555 | S_IFDIR;
    vfat_info.root_inode.st_nlink = 1;
    vfat_info.root_inode.st_uid = vfat_info.mount_uid;
    vfat_info.root_inode.st_gid = vfat_info.mount_gid;
    vfat_info.root_inode.st_size = 0;
    vfat_info.root_inode.st_atime = vfat_info.root_inode.st_mtime = vfat_info.root_inode.st_ctime = vfat_info.mount_time;

    size_t t = (lseek(vfat_info.fd, 0, SEEK_END)-vfat_info.cluster_begin_offset)/vfat_info.cluster_size;

}

