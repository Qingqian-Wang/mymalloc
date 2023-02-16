#ifndef MYMALLOC_H
#define MYMALLOC_H
// #define offset 16777216
#include <stdlib.h>


//==================================================================================================
// the block sturcture



// the head and tail pointer respectively points at the first and the last block in the memory
void *head = NULL;
void *tail = NULL;
void* firstFreedBlock = NULL;
// =================================some helper funcitons ============================================

// Funtion: identify weather the current block is valid
// return value: return 1 when valid, return 0 when invalid
size_t getTheSize(void* blockPtr);

// Funtionality: return the usable size of the block
int getIsValid(void* blockPtr);


// Function: clean the memory and set the addtional information of the block
// Paramater: 
//           size: the size of the block you want to fill in the info head
//           isValid: 0 means the current block is invalid, 1 means valid
//           tempPtr: the starting address of the block
//           keepTheSize : 1 means keep the original size, paramater 'size' is invalid in this case,  0 means not
void fillTheSize(size_t size, void *tempPtr, int isValid, int keepTheSize);

// Function: check weather the following adjacent block are both valid, if so merge them into one block
// Return value: NULL when failed, return the starting address of the new block when success
void* checkAndMerge(void *leftBlockPtr, void* rightBlockPtr);



// Funtion: try to move to the next block
// Return value: return NULL when failed to move to the next block 
void *moveToNextBlock(void *blockPtr);

// Funtion: try to move to the previous block
// Return value: return NULL when failed to move to the previous block 
void *moveToPreBlock(void *blockPtr);


// Funtion: try to allocate a new block from the previous block, and implicitly break the block if the size is enough
// Return value: return 0 when failed, return 1 when successful
int tryToFillInBlock(void *blockPtr, size_t newSize);




//=======================the end of the help funtion==========================================
//=====================the main part of the malloc and free====================================
void *ff_malloc(size_t newSize);

void *bf_malloc(size_t  newSize);

void ff_free(void *dataPtr);

void bf_free(void *dataPtr);

//=============================the end of the main functions===========================

//==========unctions to get the main information about the data segment ====================
// Return the largest size of the freed blocks
unsigned long get_largest_free_data_segment_size();

// Return the size of the total memory been allocated
unsigned long get_total_free_size();

#endif