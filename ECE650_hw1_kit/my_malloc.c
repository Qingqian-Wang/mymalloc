#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "my_malloc.h"
#define offset 16777216


//==================================================================================================
// the block sturcture
// blockHead 4 bytes, blockTail 4 bytes(the first bytes store the isValid and isTail,
// the last three bytes store the size of the block, the tail and the head is not included)

// =================================some help funcitons ============================================
size_t getTheSize(void* blockPtr){
    int *temp = (int*)blockPtr;
    return (*temp) % offset;
}

int getIsValid(void* blockPtr){
    int *temp = (int*)blockPtr;
    return (*temp) / offset;
}
// this funtion is used for initailze a new block, either from the sbrk or the freeed memory
// Functionality: clean the memory and set the head and tail
// isValid: 0 means unvalid, 1 means valid,
//          2 means last block and valid, 3 means last block and unvalid
// tempPtr here is the real begining of the block
// keepTheSize : 1 means keep the original size, para size is invalid in this case,  0 means not
void fillTheSize(size_t size, void *tempPtr, int isValid, int keepTheSize)
{
    int newSize = size;
    if(keepTheSize){
        newSize = getTheSize(tempPtr);
    }
    memset(tempPtr, 0, newSize + 8);
    int *blockTail = (int *)(tempPtr + newSize + 4);
    int *blockHead = (int *)tempPtr;
    *blockHead = newSize + isValid * offset;
    *blockTail = newSize + isValid * offset;
}

// need change the tail when merge the tail with other block
// return 0 when failed, return the begining of the new block when success
void* checkAndMerge(void *leftBlockPtr, void* rightBlockPtr)
{
    if(getIsValid(leftBlockPtr) && getIsValid(rightBlockPtr)){
        size_t newSize = getTheSize(leftBlockPtr) + getTheSize(rightBlockPtr);
        fillTheSize(newSize + 8, leftBlockPtr, 1, 0);
        if(tail == rightBlockPtr){
            tail = leftBlockPtr;
        }
        return leftBlockPtr;
    }
    return NULL;
}



// return NULL when try to move to the next block of the tail block
void *moveToNextBlock(void *blockPtr)
{
    if (blockPtr == tail)
    {
        return NULL;
    }
    size_t size = getTheSize(blockPtr);
    return blockPtr + 8 + size;
}

// return NULL when try to move to the previous block of the head block
void *moveToPreBlock(void *blockPtr)
{
    if (blockPtr == head)
    {
        return NULL;
    }
    size_t size = getTheSize(blockPtr - 4);
    return blockPtr - 8 - size;
}

// return 0 when failed, return 1 when success and implicitly break the block if the size is enough
int tryToFillInBlock(void *blockPtr, size_t newSize)
{
    size_t size = getTheSize(blockPtr);
    int isvalid = getIsValid(blockPtr);
    if (isvalid && size >= newSize)
    {
        // if the rest of the block is able to store the size info then break the block, otherwise return the original block
        if (size >= newSize + 8)
        {
            fillTheSize(newSize, blockPtr, 0, 0);
            void* next = blockPtr + newSize + 8;
            // void *next = moveToNextBlock(blockPtr);
            if (blockPtr == tail)
            { // update the ptr when try to break the tail
                tail = next;
            }
            fillTheSize(size - newSize - 8, next, 1, 0);
        }else{
            fillTheSize(0, blockPtr, 0, 1);
        }
        return 1;
    }
    return 0;
}


//=======================the end of the help funtion==========================================
//=====================the main part of the malloc and free====================================
void *ff_malloc(size_t newSize)
{
    // find the fitted block in the data segment
    if (head != NULL)
    {
        void* blockPtr = NULL;
        if(firstFreedBlock){
            blockPtr = firstFreedBlock;
        }else{
            blockPtr = head;
        }
        while (1)
        {
            if (tryToFillInBlock(blockPtr, newSize))
            {
                if(firstFreedBlock != NULL && firstFreedBlock == blockPtr && firstFreedBlock != tail){
                    firstFreedBlock = moveToNextBlock(firstFreedBlock);
                    while(1){
                        if(getIsValid(firstFreedBlock) == 1){
                            break;
                        }
                        if(firstFreedBlock == tail){
                            break;
                        }
                        firstFreedBlock = moveToNextBlock(firstFreedBlock);
                    }
                }
                return blockPtr + 4;   // if find a fitted block then return here
            }
            if (blockPtr == tail)
                break;
            blockPtr = moveToNextBlock(blockPtr);
        }
    }

    // the cases which a new block has been inserted at the end of the list
    void *tempPtr = sbrk(newSize + 8);
    fillTheSize(newSize, tempPtr, 0, 0);
    tail = tempPtr;
    // initialize the head block
    if (head == NULL)
    {
        head = tempPtr;
    }
    return tempPtr + 4;   
}

void *bf_malloc(size_t  newSize){
    if(head != NULL){
        int minSize = INT_MAX;
        void* targetBlock = NULL;
        void* blockPtr = NULL;
        if(firstFreedBlock){
            blockPtr = firstFreedBlock;
        }else{
            blockPtr = head;
        }
        while (1)
        {
            int tempSize = getTheSize(blockPtr);
            if(getIsValid(blockPtr) && tempSize >= newSize && tempSize < minSize){
                minSize = tempSize;
                targetBlock = blockPtr;
                if(tempSize == newSize){
                    break;
                }
            }
            if(blockPtr == tail){ // stop loop when encounter the tail block
                break;
            }
            blockPtr = moveToNextBlock(blockPtr);
        }
        if(firstFreedBlock != NULL && firstFreedBlock == targetBlock && firstFreedBlock != tail){
            firstFreedBlock = moveToNextBlock(firstFreedBlock);
            while(1){
                if(getIsValid(firstFreedBlock) == 1){
                    break;
                }
                if(firstFreedBlock == tail){
                    break;
                }
                firstFreedBlock = moveToNextBlock(firstFreedBlock);
            }
        }
        if(targetBlock){
            tryToFillInBlock(targetBlock, newSize);
            return targetBlock + 4; // if there is a fitted block then return it here
        }
    }
    void *tempPtr = sbrk(newSize + 8);
    fillTheSize(newSize, tempPtr, 0, 0);
    tail = tempPtr;
    // initialize the head block
    if (head == NULL)
    {
        head = tempPtr;
    }
    return tempPtr + 4; 
}

// need to set all memory to 0 when free
void ff_free(void *dataPtr)
{
    void *blockPtr = dataPtr - 4;
    void* preBlockPtr = NULL;
    void* nextBlockPtr = NULL;
    void* temp = NULL;
    fillTheSize(0, blockPtr, 1, 1);
    // try to merge to the previous block, if the previous block is unvalid or out of the data segment it will automatically change nothing
    if(preBlockPtr = moveToPreBlock(blockPtr)){
        if(temp = checkAndMerge(preBlockPtr, blockPtr)){
            blockPtr = temp;
        }
    }
    // try to merge to the next block, if the next block is unvalid or out of the data segment it will automatically change nothing
    if(nextBlockPtr = moveToNextBlock(blockPtr)){
        checkAndMerge(blockPtr, nextBlockPtr);
    }
    if(firstFreedBlock == NULL || firstFreedBlock > blockPtr){
        firstFreedBlock = blockPtr;
    }
    dataPtr = NULL;
}

void bf_free(void *dataPtr){
    void *blockPtr = dataPtr - 4;
    void* preBlockPtr = NULL;
    void* nextBlockPtr = NULL;
    void* temp = NULL;
    fillTheSize(0, blockPtr, 1, 1);
    // try to merge to the previous block, if the previous block is unvalid or out of the data segment it will automatically change nothing
    if(preBlockPtr = moveToPreBlock(blockPtr)){
        if(temp = checkAndMerge(preBlockPtr, blockPtr)){
            blockPtr = temp;
        }
    }
    // try to merge to the next block, if the next block is unvalid or out of the data segment it will automatically change nothing
    if(nextBlockPtr = moveToNextBlock(blockPtr)){
        checkAndMerge(blockPtr, nextBlockPtr);
    }
    if(firstFreedBlock == NULL || firstFreedBlock > blockPtr){
        firstFreedBlock = blockPtr;
    }
    dataPtr = NULL;
}

//=============================the end of the main functions===========================

//==========unctions to get the main information about the data segment ====================

unsigned long get_largest_free_data_segment_size(){
    void* blockPtr = head;
    size_t maxSize = 0;
    while(1){
        if(getTheSize(blockPtr) > maxSize){
            maxSize = getTheSize(blockPtr);
        }
        if(blockPtr == tail){
            break;
        }
        blockPtr = moveToNextBlock(blockPtr);
    }
    return maxSize;
}

unsigned long get_total_free_size(){
    void* blockPtr = head;
    size_t totalSize = 0;
    while(1){
        totalSize += getTheSize(blockPtr);
        if(blockPtr == tail){
            break;
        }
        blockPtr = moveToNextBlock(blockPtr);
    }
    return totalSize;
}