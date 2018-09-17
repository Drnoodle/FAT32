

//___________________________________________________________________  VFAT BOOT




//________________________________________________________  DESCRIPTION

/**
 * @file vfat_boot.h
 * @brief parse the FAT32 boot sector, verify integrity
 * and store usefull information
 * into the vfat_data : "vfat_info".
 * @author Hugo Bonnome, Athur Passuello
 */


#ifndef VFAT_BOOT_H
#define VFAT_BOOT_H




#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>



//________________________________________________________  DECLARATIONS



/**
 * @brief boot sector
 */
struct fat_boot_header {
    /* General */
    /* 0*/  uint8_t  jmp_boot[3];
    /* 3*/  char     oemname[8];
    /*11*/  uint16_t bytes_per_sector;
    /*13*/  uint8_t  sectors_per_cluster;
    /*14*/  uint16_t reserved_sectors;
    /*16*/  uint8_t  fat_count;
    /*17*/  uint16_t root_max_entries;
    /*19*/  uint16_t total_sectors_small;
    /*21*/  uint8_t  media_info;
    /*22*/  uint16_t sectors_per_fat_small;
    /*24*/  uint16_t sectors_per_track;
    /*26*/  uint16_t head_count;
    /*28*/  uint32_t fs_offset;
    /*32*/  uint32_t total_sectors;
    /* FAT32-only */
    /*36*/  uint32_t sectors_per_fat;
    /*40*/  uint16_t fat_flags;
    /*42*/  uint16_t version;
    /*44*/  uint32_t root_cluster;
    /*48*/  uint16_t fsinfo_sector;
    /*50*/  uint16_t backup_sector;
    /*52*/  uint8_t  reserved2[12];
    /*64*/  uint8_t  drive_number;
    /*65*/  uint8_t  reserved3;
    /*66*/  uint8_t  ext_sig;
    /*67*/  uint32_t serial;
    /*71*/  char     label[11];
    /*82*/  char     fat_name[8];
    /* Rest */
    /*90*/  char     executable_code[420];
    /*510*/ uint16_t signature;
} __attribute__ ((__packed__));



/**
 * @brief A kitchen sink for all important data about filesystem
 * @author EPFL
 */
struct vfat_data {
    const char* dev;
    int         fd;
    uid_t mount_uid;
    gid_t mount_gid;
    time_t mount_time;
    uint32_t    root_cluster;
    size_t      fat_entries;
    off_t       cluster_begin_offset;
    size_t      direntry_per_cluster;
    size_t      bytes_per_sector;
    size_t      sectors_per_cluster;
    size_t      reserved_sectors;
    size_t      sectors_per_fat;
    size_t      cluster_size;
    off_t       fat_begin_offset;
    size_t      fat_size;
    struct stat root_inode;
    uint32_t*   fat; // in mem
};


/**
 * brief : singleton instance of vfat_data
 */
struct vfat_data vfat_info;




//________________________________________________________  FUNCTIONS


/**
 * @brief Load the vfat_info declared above
 * using the fat_boot_header structure and
 * the first 512 bytes from the .fat file.
 *
 * @param dev The volume to mount
 * @author Hugo Bonnome, Arthur Passuello
 */
void  vfat_init(const char *dev);



#endif