/********************************************************************************
    CSC209 Assignment 2 C and Pointers -> smalloc.c
    Start: Febuary 7th, 2015
    By: Joshua Fung
********************************************************************************/

/********************************************************************************
    This is the file smalloc.c, it includes the function mem_init(size), smalloc (),
    sfree (), and mem_clean (). These function create a custom function that
    works like malloc and free.

    mem_init (size) -> memory initialize, initialize the memory block of size,
    giving the address to mem. Also store it in the freelist.
    This block is store in freelist which give the address of the data block
    and the size of the block. This also initialize the emty allocated_list.
    If the function fails it return NULL.

    smalloc (nbytes) -> small memory allocate, goes in to the freelist 
    and finds block that match the size. If there is no block the same size,
    it will tries to find a block with a larger size and divide it into the wanted
    byte size and free block. It will store the allocated block address in the 
    alloca_list and put the new address of the free bolck back to the freelist, as
    a linked list. If the function fails it returns NULL.

    *** Possible Improvement ***
      At the moment the program is running on a linked list, in the future it is possible 
      to change it to run on a binary tree. This might possiblely imporve the search speed 
      of the lists.

      *** Problem that might arrise ***
      The freelist might be a long linked list of small block, like 1 bytes, cause by 
      sfree (). At this point the function behaves as if there is not space and return NULL.
      The smalloc will not mearge the small blocks that is continous.

    sfree (addr)  -> small free, puts the Block back to the freelist letting smalloc to allcate 
    this block again. It will not mearge any blocks that is continuous.

    mem_clean () -> memory clean, cleans the block from mem_init from by the address mem. 
    Using the free () function.
*********************************************************************************/

/******************************************************************************************
 *                                   Header files                                         *
 ******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

/******************************************************************************************
 *                                 Custom Header files                                    *
 ******************************************************************************************/
/********************************************************************************
 * This include the function prototype of mem_init, smalloc, sfree, 
 * mem_clean. It also include the struct def of Block and mmapBlock */
#include "smalloc.h"

/******************************************************************************************
 *                                   Global varables                                      *
 ******************************************************************************************/
/********************************************************************************
 * Global varables is store the memory address, the freelist head and
   the allocated_list head */
mmapBlock *mem;
Block *freelist;
Block *allocated_list;


/******************************************************************************************
 * Function prototype specific to these functions, they are not ment for other functions  *
 ******************************************************************************************/


/******************************* For smalloc ************************************/

/* Recursivly find the node with exce same size as size
 * retruns the pointer address that points to the node */
Block **find_node_size (Block **nodePointer, int size);

/* Find the first node that the size is lager than size */
Block **find_node_larger (Block **nodePointer, int size);

/* Spiltting the large block to the wanted size
 * return the pointer that points to the remining block */
Block *split_large_block (Block **nodePointerAddr, int sizeWanted);

/* Putting the block into the allocated_list */
void *alloc_to_list (Block **nodePointerAddr, Block *nextNodePointer);


/******************************* For sfree **************************************/

/* Recursivly find the node containing the memory address returned
   Returns the pointer address that points to the node */
Block **find_memory_address (Block **nodePointer, void *memAddr);

/* Moving the node back to freelist from allocated_list */
void free_to_list (Block **nodePointerAddr,  Block **freeToPosition);

/* Find the position of where the node should be intserted back 
   Retrun the block pointing to the insert position*/
Block **find_insert_position (void *memoryAddr);

/******************* Possible to add *************/
/* Merge the continous blocks */
void mearge_nodes (Block **nodePointerAddr);
int meargeable (Block **nodePointerAddr);
void remove_resize_node (Block **nodePointerAddr);

/***************************** For mem_clean ************************************/

/* Distroy node of the freelist and allocated_list */
void distroy_node (Block *nodeToDistroy);


/******************************************************************************************
 *                           Content of the functions                                     *
 ******************************************************************************************/


/********************************************************************************/
/***********************
 * The smalloc funtion *
 ***********************
  Takes size of bytes and allocated space
  also add the allcoated memory to the allocated_list
  
  Returns:
  - NULL, if failed
  - address of reserved memory 
*********************************************************************************/
void *smalloc(unsigned int nbytes) {
  Block **nodePointerAddr, *nextNodePointer;
  void *memoryAddress;
  
  // Try to find a node of the same size wanted
  nodePointerAddr = find_node_size (&freelist, nbytes);

  // If a node of the same size does not exists
  if (nodePointerAddr == NULL) {
    // Try to find a node larger tha the size wanted
    nodePointerAddr = find_node_larger (&freelist, nbytes);
    if (nodePointerAddr == NULL) {
      return NULL;
    }
    nextNodePointer = split_large_block (nodePointerAddr, nbytes);
  }
  else {
    nextNodePointer = (*nodePointerAddr)->next;
  }
  memoryAddress = alloc_to_list (nodePointerAddr, nextNodePointer);
  return memoryAddress;
}

/***********************************************************
 *             Subfunction for smalloc                     *
 ***********************************************************/

/* Recursivly find the node with exce same size as size
   Retruns the pointer that points to the node */
Block **find_node_size (Block **nodePointer, int size) {  
  if ((*nodePointer)->size == size) {
    return nodePointer;
  }
  
  if ((*nodePointer)->next != NULL) {
    return find_node_size (&((*nodePointer)->next), size);
  }
  else {
    return NULL;
  }    
}

/* Find the first node that the size is lager than size */
Block **find_node_larger (Block **nodePointer, int size) {  
  if ((*nodePointer)->size > size) {
    return nodePointer;
  }
  
  if ((*nodePointer)->next != NULL) {
    return find_node_size (&((*nodePointer)->next), size);
  }
  else {
    return NULL;
  }    
}

/* Spilt large block
   Returns the address of the reminder block */
Block *split_large_block (Block **nodePointerAddr, int sizeWanted) {
  Block *newBlock;
  if ((newBlock = malloc (sizeof (Block))) == NULL) {
    perror("split_large_block malloc");
    exit (1);
  }
  newBlock->size = sizeWanted;
  newBlock->addr = (*nodePointerAddr)->addr;
  newBlock->next = *nodePointerAddr;

  (*nodePointerAddr)->addr = (*nodePointerAddr)->addr + sizeWanted;
  (*nodePointerAddr)->size = (*nodePointerAddr)->size - sizeWanted;
  
  *nodePointerAddr = newBlock;

  return newBlock->next;
}


/* Putting the block into the allocated_list 
   Returns the address of the memory */
void *alloc_to_list (Block **nodePointerAddr, Block *nextNodePointer) {
  Block *tempPointer;
  tempPointer = allocated_list;

  allocated_list = *nodePointerAddr;
  *nodePointerAddr = nextNodePointer;

  allocated_list->next = tempPointer;
  return (allocated_list->addr);  
}

/********************************************************************************
/**********************
* The sfree function *
**********************
  Takes the address of the allocated memory and free it by putting it 
  back to the freelist.

  Returns:
  - 0 if sucessfully putting the address back to the freelist
  - -1 if can't find the address in the allocated_list
    either because it is not allocated or it is not in the mem 
********************************************************************************/
int sfree(void *memoryAddr) {
  Block **nodePointerAddr;
  if ((nodePointerAddr = find_memory_address(&allocated_list, memoryAddr)) == NULL) {
    return -1;
  }
  
  free_to_list (nodePointerAddr, find_insert_position (memoryAddr));
  mearge_nodes (&freelist);
  mearge_nodes (&freelist);
  return 0;
}

/***********************************************************
 *             Subfunction for sfree                       *
 ***********************************************************/

/* Recursivly find the node containing the memory address returned
   Returns the pointer address that points to the node */
Block **find_memory_address (Block **nodePointer, void *memAddr) {
  if ((*nodePointer)->addr == memAddr) {
    return nodePointer;
  }
  
  if ((*nodePointer)->next != NULL) {
    return find_memory_address (&((*nodePointer)->next), memAddr);
  }
  else {
    return NULL;
  }    
}

/* Moving the node back to freelist from allocated_list */
void free_to_list (Block **nodePointerAddr, Block **freeToPosition) {
  Block *tempPointer;
  tempPointer = *nodePointerAddr;
  *nodePointerAddr = (*nodePointerAddr)->next;
  
  tempPointer->next = *freeToPosition;
  *freeToPosition = tempPointer;

  return;  
}

/* Find the position of where the node should be intserted back 
   Retrun the block pointing to the insert position*/
Block **find_insert_position (void *memoryAddr) {
  Block **nodePointer = &freelist;
  while (*nodePointer != NULL) {
    if (memoryAddr < (*nodePointer)->addr) {
      return nodePointer;
    }
    if ((*nodePointer)->next != NULL) {
      nodePointer = &((*nodePointer)->next);
    }
    else {
      return &((*nodePointer)->next);
    }
  }

  return &freelist;
}

/* Mearge thec continous node */ 
void mearge_nodes (Block **nodePointerAddr) {
  while (*nodePointerAddr != NULL) {
    
    if ((*nodePointerAddr)->next == NULL) {
      return;
    }
    if (meargeable (nodePointerAddr)) {
      remove_resize_node (nodePointerAddr);
    }
    
    nodePointerAddr = &((*nodePointerAddr)->next);
          
  }
}


int meargeable (Block **nodePointerAddr) {
  return ((*nodePointerAddr)->next->addr == (*nodePointerAddr)->addr + (*nodePointerAddr)->size);
}

void remove_resize_node (Block **nodePointerAddr) {
  Block *tempPointer;
  (*nodePointerAddr)->size += (*nodePointerAddr)->next->size;
  tempPointer = (*nodePointerAddr)->next->next;
  free ((*nodePointerAddr)->next);
  (*nodePointerAddr)->next = tempPointer;
}

  
/********************************************************************************
/*************************
 * The mem_init function *
 *************************
  The mem pointer, freelist pointer and allocated_list pointer is already created
  as global varable.

  Initalize freelist and allocated_list pointer

  Returns:
  - Nothing
 ********************************************************************************/
/* Note:  mmap is a system call that has a wide variety of uses.  In our
 *        case we are using it to allocate a large region of memory. 
 * - mmap returns a pointer to the allocated memory
 * Arguments:
 * - NULL: a suggestion for where to place the memory. We will let the 
 *         system decide where to place the memory.
 * - PROT_READ | PROT_WRITE: we will use the memory for both reading
 *         and writing.
 * - MAP_PRIVATE | MAP_ANON: the memory is just for this process, and 
 *         is not associated with a file.
 * - -1: because this memory is not associated with a file, the file 
 *         descriptor argument is set to -1
 * - 0: only used if the address space is associated with a file.
 */
void mem_init(int size) {
  if ((mem = malloc (sizeof (mmapBlock))) == NULL) {
    perror ("mem");
    exit (1);
  }
  
  // Using mmap to initialize the free block space
  mem->addr = mmap(NULL, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  // Error checking the mmap initialized space if failed
  if (mem == MAP_FAILED) {
    perror ("mmap");
    exit (1);
  }

  mem->size = size;

  // Initialize freelist, fisrt node
  if ((freelist = malloc (sizeof (Block))) == NULL) {
    perror ("Initialze freelist");
    exit (1);
  }

  freelist->addr = mem->addr;
  freelist->size = size;
  freelist->next = NULL;

  // Initialize allocated_list
  allocated_list = NULL;
}

/***********************************************************
 *             Subfunction for mem_init                    *
 ***********************************************************/

/* Nothing */


/******************************************************************************/
/**************************
 * The mem_clean function *
 **************************

  Returns:
  - Nothing
 ********************************************************************************/
void mem_clean(){
  // Recusivlly freeing all of the allocated_list node
  distroy_node (allocated_list);
  
  // Recursivly freeing all of the freelist node
  distroy_node (freelist);
  
  // Freeing the block space form mmap
  if (munmap (mem->addr, mem->size) == -1) {
    perror ("munmap");
    exit (1);
  }

  free (mem);
  mem = NULL;
}

/***********************************************************
 *             Subfunction for mem_clean                   *
 ***********************************************************/

/* Recusively distroy the nodes */
void distroy_node (Block *nodeToDistroy) {
  if (! nodeToDistroy){
    return;
  }

  distroy_node (nodeToDistroy->next);

  free (nodeToDistroy);
  nodeToDistroy = NULL;
}
  

  
