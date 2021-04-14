/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define MINBLOCKSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc)) /* Pack a size and allocated bit into a word */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p)&~0x7)
#define GET_ALLOC(p) (GET(p)&0x1)

#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp)-DSIZE))


team_t team = {
    /* Team name */
    "YYYYYY",
    /* First member's full name */
    "Yao Yin",
    /* First member's email address */
    "InYuo1997@gmail.com",
    /* Second member's full name (leave blank if none) */
    "Yao Yin",
    /* Second member's email address (leave blank if none) */
    "InYuo1997@gmail.com"
};

static char* heap_listp;
static char* prev_fit;
static void* extend_heap(size_t words);
static void* coalesce(void *bp);
static void* find_fit(size_t asize);
static void split_block(void* bp, size_t asize);
static void place(void* bp, size_t asize);
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *bp);
void *mm_realloc(void *ptr, size_t size);

/* 
 * extend heap by words * word(4 bytes)
 */
static void* extend_heap(size_t words) 
{
    /* TODO: mem_sbrk return the old_brk which is 1 byte + previous heap */
    char* bp;
    size_t size;
    //allocate an even number of words to maintain alignment
    size = (words & 1) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

/*
 * coalesce
 */
static void *coalesce(void *bp) 
{
    int flag = (bp == prev_fit);
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc) {
        // case 1: don't need to coalesce
        return bp;
    }
    else if (prev_alloc && !next_alloc) {
        // case 2: coalesce with the next free block;
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) {
        // case 3: coalesce with the prev free block;
        size += GET_SIZE((HDRP(PREV_BLKP(bp))));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else {
        // case 4: coalesce with the prev free block and the next free block;
        size += GET_SIZE((HDRP(PREV_BLKP(bp))));
        size += GET_SIZE((HDRP(NEXT_BLKP(bp))));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    if (flag) prev_fit = bp;
    return bp;
}



int mm_init(void)
{
    //mprintf("%d", SIZE_T_SIZE);
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1)
        return -1;
    PUT(heap_listp, 0); // alignment padding , payload region DWORD aligning 
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); //prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); //prologue footer
    PUT(heap_listp + 3*WSIZE, PACK(0, 1)); // epilogue header
    heap_listp += 2*WSIZE;
    prev_fit = heap_listp;
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    return 0;
}

/*
 * find_fit - use next fit strategy to find an empty block.
 */
static void* find_fit(size_t asize)
{
    for (char *bp = prev_fit; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
        {
            prev_fit = bp;
            return bp;
        }
    }
    for (char* bp = heap_listp; GET_SIZE(HDRP(bp)) > 0 && bp < prev_fit; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
        {
            prev_fit = bp;
            return bp;
        }
    }
    return NULL;
}
/*
static void *find_fit(size_t asize)
{
    // simple find first block to fit;
    void *bp = heap_listp;
    while (GET_SIZE(HDRP(bp))) {
        if (GET_ALLOC(HDRP(bp)) || GET_SIZE(HDRP(bp)) < asize) {
            bp = NEXT_BLKP(bp);
        } else {
            return bp;
        }
    }
    return NULL;
}*/
/*
 * split_block
 */
static void split_block(void* bp, size_t asize)
{
    size_t size = GET_SIZE(HDRP(bp));

    if ((size - asize) >= MINBLOCKSIZE)
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(size-asize, 0));
        PUT(FTRP(bp), PACK(size-asize, 0));
    }
}
/*
 * place - Place the request block at the beginning of the free block, 
 *         and only split if the remaining part is equal to or larger than the size of the smallest block
 */
static void place(void *bp, size_t asize) {
    size_t curr_size = GET_SIZE(HDRP(bp));
    //size_t next_size = curr_size - asize;
    PUT(HDRP(bp), PACK(curr_size, 1));
    PUT(FTRP(bp), PACK(curr_size, 1));
    /*
    *HDRP(NEXT_BLKP(bp)) = PACK(next_size, 0);
    *FTRP(NEXT_BLKP(bp)) = PACK(next_size, 0);
    coalesce(NEXT_BLKP(bp));
    */
   split_block(bp, asize);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; // Adjusted block size;
    size_t extend_size; // amount to extend the heap if can not find fit one;
    char *bp;

    if (size == 0) {
        return NULL;
    }
    if (size <= DSIZE) {
        asize = 2*DSIZE;
    } else {
        asize = DSIZE *((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    
    //extend the heap if no fit block found;

    extend_size = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extend_size/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    size = GET_SIZE(HDRP(oldptr));
    copySize = GET_SIZE(HDRP(newptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize-WSIZE);
    mm_free(oldptr);
    return newptr;
}
/*
void *mm_realloc(void *bp, size_t size)
{
    if (bp == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(bp);
        return NULL;
    }
    size_t asize = DSIZE *((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    if (asize == GET_SIZE(HDRP(bp))) {
        return bp;
    }
    // case 0: size < curr block payload size
    if (asize < GET_SIZE(HDRP(bp)) {
        place(bp, asize);
        PUT(HDRP(bp), PACK(asize, 1);
        PUT(FTRP(bp), PACK(asize, 1));
        return bp;
    }
    // case 1: next block is free block with enough size
    int tot_size = GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(bp);
    if (!GET_ALLOC(HDRP(NEXT_BLKP(bp))) && tot_size >= asize) {
        PUT(HDRP(bp), PACK(tot_size, 1));
        PUT(FTRP(bp), PACK(tot_size, 1));
        return bp;
    }
    // case 2: find new one and copy it;
    void *new_bp = mm_malloc(size);
    int curr_payload_size = GET_SIZE(HDRP(bp));

    for (int tmp = WSIZE; *bp + tmp < FTRP(bp); tmp += WSIZE) {
        PUT(new_bp + tmp, GET(bp + tmp));
    }
    return new_bp;
}*/