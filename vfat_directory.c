

#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include "vfat_directory.h"
#include "vfat_cluster.h"
#include "vfat_boot.h"


//_________________________________________________________________  PRIVATE FUNCTIONS



 /**
 * @param entry_pt A pointer to data.
 * @return A boolean value answering : is a long direntry ?
 * @author Arthur Passuello
 */
 static int is_long_dir_entry(unsigned char * entry_pt){


     struct fat32_direntry_long * direntry =
             (struct fat32_direntry_long *)entry_pt;

     return ( (direntry->attr & 0xF) == 0xF);
 }

/**
  * @param c A pointer to a character
  * @return A boolean answering :
  * is the char conform to the fat32 filename ?
  * @author Arthur Passuello
  */
 static int is_a_conform_filename_char(unsigned char * c){

     int is_valid = (*c <= 126);
     is_valid = (is_valid && (*c != 0x22));
     is_valid = (is_valid && (*c != 0x2A));
     is_valid = (is_valid && (*c != 0x2B));
     is_valid = (is_valid && (*c != 0x2C));
     is_valid = (is_valid && (*c != 0x2E));
     is_valid = (is_valid && (*c != 0x2F));
     is_valid = (is_valid && (*c != 0x3A));
     is_valid = (is_valid && (*c != 0x3B));
     is_valid = (is_valid && (*c != 0x3C));
     is_valid = (is_valid && (*c != 0x3D));
     is_valid = (is_valid && (*c != 0x3E));
     is_valid = (is_valid && (*c != 0x3F));
     is_valid = (is_valid && (*c != 0x5B));
     is_valid = (is_valid && (*c != 0x5C));
     is_valid = (is_valid && (*c != 0x5D));
     is_valid = (is_valid && (*c != 0x7C));

     return is_valid;
 }


 /**
 * @param direntry A pointer to a fat32_direntry.
 * @return A boolean value answering : is a valid direntry ?
 * @author Hugo Bonnome
 */
 static int is_a_valid_dir_entry_short(struct fat32_direntry * direntry){

     int is_valid = 1;
     is_valid = direntry->name[0] != 0xE5;
     is_valid = (is_valid && (direntry->name[0] != 0x00));
     is_valid = (is_valid && (direntry->name[0] != 0x20));

     int i = 0;
     while (i < 11) {
         is_valid = (is_valid && is_a_conform_filename_char((unsigned char *)&direntry->name[i]));
         i++;
     }


     return is_valid;
 }


/**
 * @param direntry A pointer to a direntry_long.
 * @return A boolean value answering : is a valid direntry_long ?
 * @author Hugo Bonnome
 */
 static int is_a_valid_dir_entry_long(
         struct fat32_direntry_long * direntry,
         unsigned char expected_checksum){

    int is_valid = 1;
    is_valid = (is_valid & ((direntry->attr & 0xF) == 0xF));
    is_valid = (is_valid & (direntry->csum == expected_checksum));

    return is_valid;
 }


/**
 * @brief compute The checksum of the filename.
 * @param pFcbName The name to be hashed
 * @return The checksum of the string
 * @author Microsoft
 */
static unsigned char checksum(unsigned char *pFcbName){

    short fcbNameLen = 11;

    unsigned char sum = 0;

    while(fcbNameLen!=0){
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *pFcbName++;
        fcbNameLen--;
    }

    return sum;
}


 /**
  * @param short_dir_pt A pointer to the short direntry representing the long direntry.
  * @param parsed_name An allocated buff of char that will contain the name of the long direntry.
  * @return A boolean answering : is an error during parsing process ?
  * @brief load the long name of the short direntry given as parameter.
  * @author Hugo Bonnome
  */
static int parse_long_name(
        unsigned char const * const short_dir_pt,
        char * const parsed_name){


     struct fat32_direntry * short_direntry = (struct fat32_direntry *)short_dir_pt;

     unsigned char expected_checksum = checksum((unsigned char *)short_direntry->nameext);

     //take previous direntry and cast as direntry long
     struct fat32_direntry_long * direntry_long =
             (struct fat32_direntry_long *)(short_direntry-1);

     int hasError = 1;
     int str_length = 0;


     while(is_long_dir_entry((unsigned char *)direntry_long)){

         if(!is_a_valid_dir_entry_long(direntry_long, expected_checksum)){
             direntry_long--;
             continue;
         }
         hasError = 0;

         int i = 0;
         while(i<5){
             parsed_name[str_length] = (char)direntry_long->name1[i];
             str_length +=1;
            i++;
         }

         i = 0;
         while(i<6){
             parsed_name[str_length] = (char)direntry_long->name2[i];
             str_length +=1;
             i++;
         }

         i = 0;
         while(i<2){
            parsed_name[str_length] = (char)direntry_long->name3[i];
            str_length +=1;
             i++;
         }

         direntry_long--;
     }

	 parsed_name[str_length] = '\0';

     return hasError;
}



/**
 * @param name a pointer to the name
 * @brief remove useless space at the end of the string
 * @author Arthur Passuello
 */
static void normalize_name(char * name){

    int currNameLength = strlen(name);
    int i = currNameLength-1;
    while(i>=0 && name[i] == 0x20){
        i--;
    }
    name[i+1] = '\0';

}


/**
 * @param date a fat32 date
 * @param time a fat32 time
 * @return a date converted in unix style
 * @author Hugo Bonnome
 */
static uint32_t parse_date(uint16_t date, uint16_t time){

    uint32_t result = 0;

    uint32_t days = (date & 0x001F)-1; // => range [0 : 30]
    uint32_t months = ((date & 0x01E0) >> 5) - 1; // => range [0 : 11]
    uint32_t years = (date & 0xFE00) >> 9;

    uint32_t secs = time & 0x001F;
    uint32_t mins = (time & 0x07E0) >> 5;
    uint32_t hours = (time & 0xF800) >> 11;

    int total_leap_year = 0;
    int i = 0;
    // converts the years from unix Epoch(1980) to fat32 Epoch(1970)
    while(i<years+10){
        int tested_year = i + 1970;
        // and count leap years following the formula below
        if(tested_year%4==0 && (tested_year%100 != 0 || tested_year%400 == 0) ){
            total_leap_year++;
        }

        i++;
    }

    result += (years+10)*365*24*3600;
    result += months*30*24*3600;
    result += (days+total_leap_year)*24*3600;
    result += secs*2;
    result += mins*60;
    result += hours*3600;
    result -= 3600;


    return result;
}


 //_________________________________________________________________  PUBLIC FUNCTIONS


 int vfat_readdir(
         uint32_t first_cluster,
         fuse_fill_dir_t callback,
         void *callbackdata) {

     // stat info about an entry
     struct stat st;
     // long name (if LFN)
     unsigned char entryName[257];
     // all entity has at least one short direntry
     struct fat32_direntry * short_direntry;
     // cast the callbackdata
     struct vfat_search_data * searched_entry = (struct vfat_search_data *)callbackdata;


     // allocate mem to load the needed clusters
     int total_cluster_loaded = cluster_chain_size(first_cluster);

     unsigned char * data = calloc(total_cluster_loaded, vfat_info.cluster_size);
     if( load_cluster_chain(data, first_cluster) != total_cluster_loaded){
         return 1;
     }


     // iterate over direntry
     unsigned char * entry_pt = data;
     // (used to avoid deadlock if the end of dir is corrupted)
     int total_direntry_loaded = 0;

     while(entry_pt[0] != 0x00 && total_direntry_loaded<total_cluster_loaded*vfat_info.direntry_per_cluster) {

         int totalDirentryLoadedTest = (entry_pt-data)/32;

         if(is_long_dir_entry(entry_pt)){

             // locate next short entry (related one)
             while(is_long_dir_entry(entry_pt)){
                 entry_pt += 32;
                 total_direntry_loaded++;
             }

             // entry_pt_clone is on the representative short entry
             short_direntry = (struct fat32_direntry *)entry_pt;

             // the representative short isn't ok : ignore
             if(!is_a_valid_dir_entry_short(short_direntry)){
                 entry_pt += sizeof(struct fat32_direntry);
                 continue;
             }

             // extract long name
             int hasError = parse_long_name(entry_pt, entryName);

             // increment pointer to next entity
             entry_pt += sizeof(struct fat32_direntry);
             total_direntry_loaded++;

             if(hasError){
                 continue;
             }

             normalize_name(entryName);

         }
         else if( (((struct fat32_direntry *)entry_pt)->attr & 0x02) == 0x02 ){
             // is an hidden file
             total_direntry_loaded += 1;
             entry_pt += 32;
             continue;
         }
         else {

             short_direntry = (struct fat32_direntry *)entry_pt;

             // increment pointer to next entity
             total_direntry_loaded += 1;
             entry_pt += 32;
             if(!is_a_valid_dir_entry_short(short_direntry)){
                 continue;
             }

             int i = 0;
             while(i<8) {
                 entryName[i] = short_direntry->name[i];
                 i++;
             }
             entryName[i] = '\0';

             normalize_name(entryName);

             if(short_direntry->ext[0] >= 65 && short_direntry->ext[0] <= 90) {

                 int nameLength = strlen(entryName);

                 entryName[nameLength] = '.';
                 entryName[nameLength + 1] = short_direntry->ext[0];
                 nameLength+=2;

                 if(short_direntry->ext[1] >= 65 && short_direntry->ext[1] <= 90) {
                     entryName[nameLength] = short_direntry->ext[1];
                     nameLength++;
                 }
                 if(short_direntry->ext[2] >= 65 && short_direntry->ext[2] <= 90) {
                     entryName[nameLength] = short_direntry->ext[2];
                     nameLength++;
                 }

                 entryName[nameLength] = '\0';
             }
         }



         // HERE :
         // short direntry is valid and parametrized in the shortdirentry pt
         // longName contains empty string or the name of the file in case of LFN
         // the entry_pt is parametrized to the next entity

         // mark cluster id to reach entry in the vfat search

         searched_entry->cluster_id_current = (((uint32_t)short_direntry->cluster_hi << 16)
                                              | (uint32_t)short_direntry->cluster_lo);


         // instanciate struct stat (file info used by os)
         memset(&st, 0, sizeof(st));
         st.st_dev = 0;
         st.st_ino = searched_entry->cluster_id_current;
         st.st_mode = S_IRWXU | S_IRWXG | S_IRWXO;
         st.st_nlink = 1;
         st.st_uid = vfat_info.mount_uid;
         st.st_gid = vfat_info.mount_gid;
         st.st_rdev = 0;
         st.st_size = short_direntry->size;
         st.st_blksize = 0;
         st.st_blocks = 1;


         st.st_atime = parse_date(short_direntry->atime_date, 0);
         st.st_mtime = parse_date(short_direntry->mtime_date, short_direntry->mtime_time);
         st.st_ctime = parse_date(short_direntry->ctime_date,short_direntry->ctime_time)+ short_direntry->ctime_ms/100;


         if((short_direntry->attr & 0x10) == 0x10){ // if is dir
             st.st_mode = st.st_mode | S_IFDIR;
         }
         else {
             st.st_mode = st.st_mode | S_IFREG; // File
         }



         off_t offset = 0;
         callback(callbackdata,entryName,&st,offset);

     }

     free(data);

     return 0;
 }


 int vfat_search_entry(
         void *data,
         const char *name,
         const struct stat *st,
         off_t offs) {


    struct vfat_search_data *sd = data;

    // entry is not the searched one
    if (strcmp(sd->name, name) != 0){
        return 0;
    }

    sd->found = 1;
    *sd->st = *st;
    sd->cluster_id_found = sd->cluster_id_current;

     return 1;
 }


 int vfat_resolve(const char *path, struct stat *st)
 {

     int res = -ENOENT; // Not Found
     if (strcmp("/", path) == 0) {
         *st = vfat_info.root_inode;
         return 0;
     }

     char pathTokenized[strlen(path)+1];
     strcpy(pathTokenized, path);

     const char delim[2] = "/";
     char * token = NULL;
     token = strtok(pathTokenized,delim);

     struct vfat_search_data search_req;

     search_req.st = malloc(sizeof(struct stat));
     search_req.cluster_id_found =  vfat_info.root_cluster;

     while(token != NULL){

         search_req.name = token;
         search_req.found = 0;

         vfat_readdir(search_req.cluster_id_found,&vfat_search_entry,&search_req);
         token = strtok(NULL, delim);

         if(!search_req.found){
             return - errno;
         }

     }

     if(search_req.found){
         res = 0;
     }

     *st = *search_req.st;
     free(search_req.st);

     return res;
 }

