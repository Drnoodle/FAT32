



//_________________________________________________________________  PRIVATE FUNCTIONS


#include <assert.h>
#include <endian.h>
#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include "vfat_boot.h"
#include "vfat_cluster.h"

/**
 * @param cluster_id A cluster identifier.
 * @return A boolean value indicating that
 * the cluster identifier given as parameter is the last element
 * of the fat chain.
 * @author Hugo Bonnome
 */
static int is_end_of_cluster_chain(uint32_t cluster_id){
    return ((cluster_id & 0x0FFFFFFF) >= 0x0FFFFFF0);
}


/**
 * @param buff An allocated pointer that will store
 * the content of a cluster as an array of unsigned char
 * @param cluster_id A cluster identifier
 * @return boolean value answering : has error during the operation ?
 * @author Arthur Passuello
 */
static int load_cluster(unsigned char * buff, uint32_t cluster_id){

    size_t offset =
            vfat_info.cluster_begin_offset
            + (cluster_id-vfat_info.root_cluster)*vfat_info.sectors_per_cluster*vfat_info.bytes_per_sector;

    int total = pread(vfat_info.fd, buff, vfat_info.cluster_size,offset);

    return total != vfat_info.cluster_size;
}


//_________________________________________________________________  PUBLIC FUNCTIONS




int cluster_chain_size(uint32_t cluster_id){

    uint32_t curr_clust_id = cluster_id;

    int i = 0;
    while(!is_end_of_cluster_chain(curr_clust_id)){
        i++;
        curr_clust_id = vfat_next_cluster(curr_clust_id);
    }

    return i;
}


int load_cluster_chain(unsigned char * buff, uint32_t cluster_id){

    size_t cluster_chain_size=0;
    uint32_t curr_clust_id = cluster_id;

    while(!is_end_of_cluster_chain(curr_clust_id)){

        int reading_error = load_cluster(buff,curr_clust_id);

        if(reading_error){
            return -1;
        }

        buff = buff + vfat_info.cluster_size;
        curr_clust_id = vfat_next_cluster(curr_clust_id);
        cluster_chain_size++;
    }

    return cluster_chain_size;
}



int vfat_next_cluster(uint32_t cluster_id)
{
    return vfat_info.fat[cluster_id];
}





