/********************************************************************************
    CSC209 Assignment 2 C and Pointers -> smalloc.h
    Start: Febuary 7th, 2015
    By: Joshua Fung
********************************************************************************/
/* Include gard for the header file, defineing SMALLOC_H */
# ifndef SMALLOC_H
# define SMALLOC_H

// The node for the freelist and allocated_list
struct block {
  void *addr; /*start address of memory for this block */
  int size;
  struct block *next;
};
typedef struct block Block;

/********************************************************************************
 I have decided to create a stuct to keep track of the addrees and the 
 size of the block for the purpose for munmap **
********************************************************************************/
typedef struct mmapblock {
  void *addr;
  int size;
} mmapBlock;

/*******************************************************************************/
/****************************
 * Implemented in smalloc.c *
 ****************************/

/* Allocates size bytes of memory for the dynamically allocated memory 
 * algorithm to use */
void mem_init(int size);

/* Reserves nbytes of space from the memory region created by mem_init.  Returns
 * a pointer to the reserved memory. Returns NULL if memory cannot be allocated */    
void *smalloc(unsigned int nbytes);

/* Free the reserved space starting at addr.  Returns 0 if successful 
 * -1 if the address cannot be found in the list of allocated blocks */
int sfree(void *memoryAddr);

/* Free any dynamically used memory in the allocated and free list */
void mem_clean();

# endif
