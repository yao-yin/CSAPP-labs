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

// useful macros

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
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

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = (val))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE*3)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE*2)

#define LL_PREV(bp) ((char *)(bp) - WSIZE*2)
#define LL_NEXT(bp) ((char *)(bp) - WSIZE)
#define GET_LL_PREV(bp) (*(void **)LL_PREV(bp))
#define GET_LL_NEXT(bp) (*(void **)LL_NEXT(bp))

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp)-DSIZE*2))

/* 
 * mm_init - initialize the malloc package.
 */
static void *heap_listp;

static void *extend_heap(size_t words);
static void *mm_malloc(size_t size);
static void *mm_free(void * bp);
int mm_init(void);

static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;
    size = (words & 1) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }
    
}

int mm_init(void)
{
    if ((heap_listp = mem_sbrk(6*WSIZE)) == (void *) - 1)
        return -1;
    PUT(heap_listp, 0); //padding
    /* prologue block */
    PUT(heap_listp + (1 * WSIZE), PACK(4*WSIZE, 1)); // header
    PUT(heap_listp + (2 * WSIZE), 0); // ptr to prev(NULL)
    PUT(heap_listp + (3 * WSIZE), (unsigned int)(heap_listp + 5*WSIZE)); // ptr to epilogue block
    PUT(heap_listp + (4 * WSIZE), PACK(4* WSIZE, 1)); // footer
    PUT(heap_listp, 1);
    heap_listp += 4*WSIZE;
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
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
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














