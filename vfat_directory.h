

//___________________________________________________________________  VFAT DIRECTORY







//________________________________________________________  DESCRIPTION

/**
 * @file vfat_directory.h
 * @brief Provides tools helping in the task
 * of extracting information from a directory.
 * @Author Hugo Bonnome, Arthur Passuello
 */


#ifndef VFAT_DIRECTORY_H
#define VFAT_DIRECTORY_H


#include <fuse.h>







//________________________________________________________  DECLARATIONS


/**
 * @brief A fat32 direntry (short)
 */
struct fat32_direntry {
    /* 0*/  union {
        struct {
            char name[8];
            char ext[3];
        };
        char nameext[11];
    };
    /*11*/  uint8_t  attr;
    /*12*/  uint8_t  res;
    /*13*/  uint8_t  ctime_ms;
    /*14*/  uint16_t ctime_time;
    /*16*/  uint16_t ctime_date;
    /*18*/  uint16_t atime_date;
    /*20*/  uint16_t cluster_hi;
    /*22*/  uint16_t mtime_time;
    /*24*/  uint16_t mtime_date;
    /*26*/  uint16_t cluster_lo;
    /*28*/  uint32_t size;
} __attribute__ ((__packed__));

#define VFAT_ATTR_DIR   0x10
#define VFAT_ATTR_LFN   0xf
#define VFAT_ATTR_INVAL (0x80|0x40|0x08)



/**
 * @brief A fat32 direntry (long)
 */
struct fat32_direntry_long {
    /* 0*/  uint8_t  seq;
    /* 1*/  uint16_t name1[5];
    /*11*/  uint8_t  attr;
    /*12*/  uint8_t  type;
    /*13*/  uint8_t  csum;
    /*14*/  uint16_t name2[6];
    /*26*/  uint16_t reserved2;
    /*28*/  uint16_t name3[2];
} __attribute__ ((__packed__));

#define VFAT_LFN_SEQ_START      0x40
#define VFAT_LFN_SEQ_DELETED    0x80
#define VFAT_LFN_SEQ_MASK       0x3f



/**
 * @brief A search entry used to querry a file or
 * a subfolder in a given folder.
 */
struct vfat_search_data {
    const char*  name;
    uint32_t cluster_id_current;
    uint32_t cluster_id_found;
    int found;
    struct stat* st;
};





//________________________________________________________  PUBLIC FUNCTIONS



/**
 * @param first_cluster The cluster identifier related to the first
 * cluster of a directory.
 * @param callback A function (void*, const char*,const struct stat *, void*) => int
 * @param callbackdata A pointer to something
 * @return A boolean answering : is error during the process ?
 * @author Hugo Bonnome
 */
int vfat_readdir(uint32_t first_cluster, fuse_fill_dir_t callback, void *callbackdata);


/**
 * @param data A pointer to a vfat_search_data(request).
 * @param name The file name of a file tested where we're currently looking.
 * @param st A pointer to a struct stat that contains informations about the file tested
 * @param offs The offset of the file tested in the current directory (in bytes)
 * @return A boolean answering : is the request equals to the tested file ?
 * @author Arthur Passuello
 */
int vfat_search_entry(void *data, const char *name, const struct stat *st, off_t offs);


/**
 * @param path The path to a requested file
 * @param st A pointer to a stat that will contains informations of the request.
 * @return A boolean answering : is st correctly loaded (~search succeed) ?
 * @author Hugo Bonnome
 */
int vfat_resolve(const char *path, struct stat *st);




#endif