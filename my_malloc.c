#include "my_malloc.h"

unsigned long data_segment_size = 0; // sbrk space
unsigned long data_segment_free_space_size = 0; //rest space

block * headBlock = NULL;



block * allocate(size_t size) {
    void * new = sbrk(size + sizeof(block));
    // assert(new != (void*) - 1);
    if (new == (void*)-1){
      return NULL;
    }
    block * last = new;
    last->next = NULL;
    last->prev = NULL;
    last->blockSize = size;
    data_segment_size += size + sizeof(block); //---

    return last;
}


block * split(block * ptr, size_t size){
    block * newblock = (void*)ptr + size + sizeof(block);

    if (ptr->next){
        newblock->next = ptr->next;
        ptr->next->prev = newblock;

        ptr->next = newblock;
        newblock->prev = ptr;
    }
    else{
        ptr->next = newblock;
        newblock->prev = ptr;
        newblock->next = NULL;
    }
    newblock->blockSize = ptr->blockSize - size - sizeof(block);

    ptr->blockSize = size;
    return ptr;
}



block * ff_find(size_t size){
    block * curr = headBlock;
    block * target = NULL;

    if (headBlock == NULL){
        return NULL;
    }
    else if (headBlock->blockSize >= size){ ///------------
        return headBlock;
    }
    else{
        while (curr && curr->blockSize < size){
            // find the memory
            curr = curr->next;
            target = curr;
        }
        return target;
    }
}

void removeBlock(block * ptr){
    if (ptr->prev && ptr->next){
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
        ptr->prev = NULL;
        ptr->next = NULL;
    }
    else if (ptr->prev == NULL && ptr->next){
        block * new = ptr->next;
        ptr->next->prev = NULL;
        ptr->next = NULL;
        headBlock = new;
    }
    else if (ptr->prev && ptr->next == NULL){
        ptr->prev->next = NULL;
        ptr->prev = NULL;
    }
    else{
        headBlock = NULL;
    }
}

void * ff_malloc(size_t size){

    block * ptr = NULL;

    if (headBlock == NULL){
        ptr = allocate(size);
    }
    else{
        ptr = ff_find(size); // >= size or NULL

        if (ptr == NULL){
            ptr = allocate(size);
        }
        else if (ptr->blockSize > size + sizeof(block)){
            ptr = split(ptr, size);
            removeBlock(ptr);
        }

        else if(ptr->blockSize >= size && ptr->blockSize <= size + sizeof(block)){
            removeBlock(ptr);
        }
    }

    return (void*)ptr + sizeof(block); // for user
}


void merge(block * ptr){
    ptr->blockSize += ptr->next->blockSize + sizeof(block);
    removeBlock(ptr->next);
}


void ff_free(void * ptr){

    // no freelist at the beginning
    if (headBlock == NULL){

        headBlock = (void*)ptr - sizeof(block);
        headBlock->next = NULL;
        headBlock->prev = NULL;

    }
    else{
        // first block
        block * curr = headBlock;

        // add block
        block * newblock = (void*)ptr - sizeof(block);

        // add before headBlock
        if (curr > newblock){
            newblock->next = curr;
            curr->prev = newblock;
            headBlock = newblock;
            newblock->prev = NULL;
        }

        else{
            while (curr->next && curr->next < newblock){
                curr = curr->next;
            }

            if (curr->next){
                newblock->prev = curr;
                curr->next->prev = newblock;
                newblock->next = curr->next;
                curr->next = newblock;
            }
            else{
                curr->next = newblock;
                newblock->prev = curr;
                newblock->next = NULL;
            }
        }

        // merge the free blocks
        if (
                (newblock->prev && (void *)newblock == (void *)newblock->prev + newblock->prev->blockSize + sizeof(block)) &&
                ((newblock->next && (void *)newblock->next > (void *)newblock + newblock->blockSize + sizeof(block)) || newblock->next == NULL)){

            merge(newblock->prev);
            // printf("finish merge left\n");
        }
        else if(
                (newblock->next && (void *)newblock->next == (void *)newblock + newblock->blockSize + sizeof(block)) &&
                ((newblock->prev && (void *)newblock > (void *)newblock->prev + newblock->prev->blockSize + sizeof(block)) || newblock->prev == NULL)){

            merge(newblock);
            // printf("finish merge right\n");
        }

        else if (
                newblock->prev &&
                newblock->next &&
                (void *)newblock == (void *)newblock->prev + newblock->prev->blockSize + sizeof(block) &&
                (void *)newblock->next == (void *)newblock + newblock->blockSize + sizeof(block)){

            merge(newblock);
            merge(newblock->prev);
            // printf("finish merge left and right\n");
        }
    }
}



block * bf_find(size_t size){
    block * curr = headBlock;
    block * target = NULL;

    while (curr){
      if (target == NULL && curr->blockSize > size){
        target = curr;
      }

      else if (curr->blockSize < target->blockSize && curr->blockSize > size){
        target = curr;
      }

      else if (curr->blockSize == size){
        target = curr;
        break;
      }
      curr = curr->next;
    }
    return target;
}



void * bf_malloc(size_t size){

    block * ptr = NULL;

    if (headBlock == NULL){
        ptr = allocate(size);
    }
    else{
        ptr = bf_find(size); // >= size or NULL

        if (ptr == NULL){
            ptr = allocate(size);
        }
        else if (ptr->blockSize > size + sizeof(block)){
            ptr = split(ptr, size);
            removeBlock(ptr);
        }

        else if(ptr->blockSize >= size && ptr->blockSize <= size + sizeof(block)){
            removeBlock(ptr);
        }
    }

    return (void*)ptr + sizeof(block); // for user
}


void bf_free(void * ptr){
  ff_free(ptr);
}


unsigned long get_data_segment_size(){
    return data_segment_size;
}

unsigned long get_data_segment_free_space_size(){
    block * freehead = headBlock;
    while (freehead){
        // printf("address and size are %p and %ld\n", (void*)freehead, freehead->blockSize);
        data_segment_free_space_size += freehead->blockSize + sizeof(block);
        freehead = freehead->next;
    }
    return data_segment_free_space_size;
}