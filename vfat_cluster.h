

//___________________________________________________________________  VFAT CLUSTER




//________________________________________________________  DESCRIPTION

/**
 * @file vfat_cluster.h
 * @brief Provides tools helping in the task of
 * reading cluster from a fat32 volume.
 * @author Hugo Bonnome, Athur Passuello
 */


#ifndef VFAT_CLUSTER_H
#define VFAT_CLUSTER_H


#include <stdint.h>




//________________________________________________________  PUBLIC FUNCTIONS


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





#endif