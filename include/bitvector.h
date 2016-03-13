
typedef unsigned int bitvector_entry;

typedef struct _bitvector {
    unsigned int num_entries;
    bitvector_entry *region;
} bitvector;

/*
 * bitvector_size - returns the size required for the bit vector region based off
 *                  the number of entries provided. rounds the size up to the nearest
 *                  bitvectory_entry size
 */
unsigned int bitvector_size(unsigned int num_of_entries);

/*
 * bitvector_init - initializes bv based off the number of entries and the
 *                  caller-allocated region
 */
void bitvector_init(unsigned int num_of_entries, void *region, bitvector *bv);

/*
 * bitvector_find - given bit_ofs, find the bit vector entry pointer and the
 *                  bit offset within the entry
 */
void bitvector_find(bitvector *bv, unsigned int bit_ofs, bitvector_entry *found, unsigned int found_ofs);

/*
 * bitvector_set - set the bit corresponding to bit_ofs in bv
 */
void bitvector_set(bitvector *bv, unsigned int bit_ofs);

/*
 * bitvector_clear - clear the bit corresponding to bit_ofs in bv
 */
void bitvector_clear(bitvector *bv, unsigned int bit_ofs);

/*
 * bitvector_release - release the bitvector in bv
 */
void bitvector_release(bitvector *bv);
