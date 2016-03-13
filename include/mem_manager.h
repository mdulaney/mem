#include "bitvector.h"
#include "cpu.h"
#include "cpu_shared.h"

// Public Declarations

/*
 * mem_alloc - allocate a region of size. addr is optional. allocated is
 *             the address that was allocated.
 */
int mem_alloc(mem_vaddr addr, unsigned int size, mem_vaddr *allocated);

/*
 * mem_free - frees the region associated with addr
 */
void mem_free(mem_vaddr addr);

// TODO: next two functions are not needed if memory shadowing is implemented
/*
 * mem_vmem_read - reads an unsigned int from the simulated virtual address
 *
 */
unsigned int mem_vmem_read(mem_vaddr ptr); 

/*
 * mem_vmem_write - writes an unsigned int (val) to ptr
 *
 */
void mem_vmem_write(vaddr ptr, unsigned int val);

/*
 *
 * Data Declarations
 *
 */

typedef struct _mm_phys_descriptor {

    struct _mm_phys_descriptor *next;

    mem_phys_addr start;
    unsigned int size;
} mm_phys_descriptor;

#define MM_PHYS_DESCRIPTOR_POOL_ENTRIES (sizeof(mm_phys_descriptor) * 100)

typedef struct _mm_phys_descriptor_pool {

    struct _mm_phys_descriptor_pool *next;
    mm_phys_descriptor descriptors[MM_PHYS_DESCRIPTOR_POOL_ENTRIES];
} mm_phys_descriptor_pool;

typedef unsigned int bitmap_entry;

// Private Data Declarations
typedef struct _mm_context {

    // Pointer to the page directory
    pg_dir_entry *root;
    
    // Pointer to the emulated physical memory
    void *mem_base;

    // Size of the physical region in bytes
    unsigned int mem_size;
    
    // bitmap of the physical address map
    bitvector addr_map;

    // memory descriptor pool - an extendable pool of descriptors to use for physical
    // region allocations
    mm_phys_descriptor_pool mdp; 

} mm_context;

// Private declarations

/*
 * mm_phys_descriptor_pool_extend - adds a new pool of available memory descriptors
 *                                  marks the first entry in the pool as the pool region itself
 *
 */
int mm_phys_descriptor_pool_extend();

/*
 * mm_phys_descriptor_pool_allocate - allocate a memory descriptor from the pool
 *
 */
int mm_phys_descriptor_pool_allocate(mm_phys_descriptor *md);

/*
 * mm_phys_descriptor_pool_free - releases the descriptor back to the pool
 *                                associated with addr.
 *
 * md - the physical descriptor to free
 */
void mm_phys_descriptor_pool_free(mm_phys_descriptor *md);

/*
 * mem_alloc_phys_region - allocates a physical region from the memory map
 *
 * mem_base - base address of the physical region
 * size - the size of the physical region. if not page-aligned, then the size of
 *        the resulting region will be rounded up
 * map - memory bitmap. must contain a contigous stream of bits corresponding to
 *       the region, otherwise an error is returned.
 * md - returns a memory descriptor corresponding to the region. the caller
 *      is responsible for adding this to the active descriptor pool
 */
int mem_alloc_phys_region(void *mem_base, unsigned int size,
                          bitvector *map, mem_descriptor *md);

/*
 * mem_free_phys_region
 *
 * map - bitvector to update
 * md - memory descriptor to free. the caller is responsible for removing this
 *      from the active descriptor pool
 */
int mem_free_phys_region(bitvector *map, mem_descriptor *md);

/*
 * mem_page_tbl_allocate_virt_addr
 *
 * tbl  - traversed until an available entry is found. the entry is populated
 * addr - placed in the found pte
 * pt_slot - slot number for the found entry
 */
int mem_page_tbl_allocate_virt_addr(pg_tbl_entry *tbl,
                                    mem_phys_addr addr,
                                    unsigned int *pt_slot);
/*
 * mem_page_dir_allocate_virt_addr
 *
 * dir   - pointer to the directory entry. all page tables referenced by dir
 *         are traversed until an empty pte is found
 * vaddr - the virtual address corresponding to the paging metadata      
 */
int mem_page_dir_allocate_virt_addr(pg_dir_entry *dir,
                                    mem_phys_addr addr,
                                    mem_vaddr vaddr);

/*
 * mem_allocate_virt_addr - traverse the page tables to find an available entry
 *
 * root  -a pointer to the page directory. the paging metadata must contain an
 *        available page table entry or else this function fails
 * addr  -physical address to populate in the page tables. this function
 *        does not check to see if the addr is unique
 * vaddr -the virtual adddress corresponding to the physical address. vaddr is
 *        returned by this function
 */
int mem_allocate_virt_addr(pg_dir_entry *root,
                           mem_phys_addr addr,
                           mem_vaddr vaddr);

/*
 * mem_manager_init - initializes the memory manager. returns an error if there
 *                    is not enough room for the paging metadata.
 *
 * Performs the following initialization steps
 * 1. initialize the address bitmap based off the amount of available RAM
 * 2. allocates physical regions for the page directory and all page tables
 * 3. updates the address map to reflect these allocations
 * 4. updates the mdp to include a descriptor for the page directory and 
 *    page table regions
 */
void mem_manager_init(void *ram_base, unsigned int ram_size);

void mem_manager_exit();
