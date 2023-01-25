#include <stdio.h>
#include <unistd.h>
#include <assert.h>

// point of a block is the tag (sizeof)
struct _block {
  struct _block * next;
  struct _block * prev;
  size_t blockSize;
};

typedef struct _block block;

unsigned long get_data_segment_size();             //in bytes
unsigned long get_data_segment_free_space_size();  //in bytes

//call sbrk
block * allocate(size_t size);

block * split(block * ptr, size_t size);

//first fit
block * ff_find(size_t size);
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//best fit
block * bf_find(size_t size);
void * bf_malloc(size_t size);
void bf_free(void * ptr);

//linkedlist operation
void merge(block * ptr);
void removeBlock(block * ptr);
