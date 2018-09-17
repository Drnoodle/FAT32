Hugo Bonnome
Arthur Passuello
_____________________________________________________   FAT32



______________________________________   TEST RESULT

5/ 5 Parsing BPB
7/ 7 Debugfs /next_cluster/num checks
7/ 7 Basic start cluster checks
30/ 30 Test different readsizes
8/ 8 File reading checks
25/ 25 Test if ls/stat linux utils work
0/ 1 Test diff -r
7/ 7 Timestamps


_____________________________________   HEADER

dbugfs.h :
Contains debugging functions.

util.h :
Contains mmap and unmap.

vfat_cluster.h :
Provides tools helping in the task of
reading cluster from a fat32 volume.

vfat_directory.h :
Provides tools helping in the task
of extracting information from a directory.

vfat_boot.h :
parse the FAT32 boot sector, verify integrity
 and store usefull information.



_____________________________________   VFAT_CLUSTER

_______________  PUBLIC FUNCTIONS


/**
 * @param cluster_id A cluster identifier.
 * @return The fat chain length
 * @author Arthur Passuello
 */
int cluster_chain_size(uint32_t cluster_id);

/**
 * @param buff An allocated pointer that will store
 * the content of a cluster chain as an array of unsigned char
 * @param cluster_id A cluster identifier
 * @return The total number of clusters successfully read.
 * @author Hugo Bonnome
 */
int load_cluster_chain(unsigned char * buff, uint32_t cluster_id);

/**
 * @param cluster_id A cluster identifier.
 * @return The Fat value entry for cluster cluster_id (i.e the next cluster id)
 * @author Hugo Bonnome
 */
int vfat_next_cluster(uint32_t cluster_id);


_______________  PRIVATE FUNCTIONS


/**
 * @param cluster_id A cluster identifier.
 * @return A boolean value indicating that
 * the cluster identifier given as parameter is the last element
 * of the fat chain.
 * @author Hugo Bonnome
 */
static int is_end_of_cluster_chain(uint32_t cluster_id);

/**
 * @param buff An allocated pointer that will store
 * the content of a cluster as an array of unsigned char
 * @param cluster_id A cluster identifier
 * @return boolean value answering : has error during the operation ?
 * @author Arthur Passuello
 */
static int load_cluster(unsigned char * buff, uint32_t cluster_id);



_____________________________________   VFAT_DIRECTORY

_______________  PUBLIC FUNCTIONS


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


_______________  PRIVATE FUNCTIONS


 /**
 * @param entry_pt A pointer to data.
 * @return A boolean value answering : is a long direntry ?
 * @author Arthur Passuello
 */
 static int is_long_dir_entry(unsigned char * entry_pt);

/**
  * @param c A pointer to a character
  * @return A boolean answering :
  * is the char conform to the fat32 filename ?
  * @author Arthur Passuello
  */
 static int is_a_conform_filename_char(unsigned char * c);

 /**
 * @param direntry A pointer to a fat32_direntry.
 * @return A boolean value answering : is a valid direntry ?
 * @author Hugo Bonnome
 */
 static int is_a_valid_dir_entry_short(struct fat32_direntry * direntry);

/**
 * @param direntry A pointer to a direntry_long.
 * @return A boolean value answering : is a valid direntry_long ?
 * @author Hugo Bonnome
 */
 static int is_a_valid_dir_entry_long(
         struct fat32_direntry_long * direntry,
         unsigned char expected_checksum);

/**
 * @brief compute The checksum of the filename.
 * @param pFcbName The name to be hashed
 * @return The checksum of the string
 * @author Microsoft
 */
static unsigned char checksum(unsigned char *pFcbName);

 /**
  * @param short_dir_pt A pointer to the short direntry representing the long direntry.
  * @param parsed_name An allocated buff of char that will contain the name of the long direntry.
  * @return A boolean answering : is an error during parsing process ?
  * @brief load the long name of the short direntry given as parameter.
  * @author Hugo Bonnome
  */
static int parse_long_name(
        unsigned char const * const short_dir_pt,
        char * const parsed_name);

/**
 * @param name a pointer to the name
 * @brief remove useless space at the end of the string
 * @author Arthur Passuello
 */
static void normalize_name(char * name);

/**
 * @param date a fat32 date
 * @param time a fat32 time
 * @return a date converted in unix style
 * @author Hugo Bonnome
 */
static uint32_t parse_date(uint16_t date, uint16_t time);



_____________________________________   VFAT_BOOT

_______________  PUBLIC FUNCTIONS
/**
 * @brief Load the vfat_info declared above
 * using the fat_boot_header structure and
 * the first 512 bytes from the .fat file.
 *
 * @param dev The volume to mount
 * @author Hugo Bonnome, Arthur Passuello
 */
void  vfat_init(const char *dev);