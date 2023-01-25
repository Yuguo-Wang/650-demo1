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


unsigned long data_segment_size = 0; // sbrk space
unsigned long data_segment_free_space_size = 0; //rest space

block * headBlock = NULL;


block * allocate(size_t size) {
    void * new = sbrk(size + sizeof(block));

    // printf("sbrk ----- address and size is %p:\n", new);

//    assert(new != (void*)-1);
    if (new == (void*)-1){
        return NULL;
    }

    block * last = new;
    last->next = NULL;
    last->prev = NULL;
    last->blockSize = size;
    data_segment_size += size + sizeof(block); //---
//    printf("sbrk ----- address and size is %p and %ld:\n", last, last->blockSize);

    // printf("new address is%p\n", new);
    // printf("finish allocate\n");
    return last;
}


block * split(block * ptr, size_t size){
    // printf("spint begin\n");

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
    // printf("finish split\n");
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
//        headBlock = ptr;
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
    // printf("finish ff_malloc\n");

    // printf("address and size: %p, %ld\n", ptr, ptr->blockSize);

    return (void*)ptr + sizeof(block); // for user
}


void merge(block * ptr){

    ptr->blockSize += ptr->next->blockSize + sizeof(block);
    removeBlock(ptr->next);

    // printf("finish merge\n");
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
                ( (newblock->next && (void *)newblock->next > (void *)newblock + newblock->blockSize + sizeof(block) ) || newblock->next == NULL)){

            merge(newblock->prev);
            // printf("finish merge left\n");
        }
        else if(
                (newblock->next && (void *)newblock->next == (void *)newblock + newblock->blockSize + sizeof(block)) &&
                ( (newblock->prev && (void *)newblock > (void *)newblock->prev + newblock->prev->blockSize + sizeof(block) ) || newblock->prev == NULL)){

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

unsigned long get_data_segment_size(){
    return data_segment_size;
}

unsigned long get_data_segment_free_space_size(){
    block * freehead = headBlock;
    while (freehead){
        printf("address and size are %p and %ld\n", (void*)freehead, freehead->blockSize);
        data_segment_free_space_size += freehead->blockSize + sizeof(block);
        freehead = freehead->next;
    }
    return data_segment_free_space_size;
}




//int main(){
//    int * arr0 = ff_malloc(sizeof(* arr0));
//    int * arr1 = ff_malloc(sizeof(* arr1));
//    int * arr2 = ff_malloc(sizeof(* arr2));
//    int * arr3 = ff_malloc(sizeof(* arr3));
//
//    ff_free(arr1);
//    ff_free(arr2);
//
//    double * arr_ = ff_malloc(sizeof(* arr_));
//
//}





//--------------------large
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define NUM_ITERS    50
#define NUM_ITEMS    1000//10000


#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)

#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p)    bf_free(p)
#endif


double calc_time(struct timespec start, struct timespec end) {
    double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
    double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;

    if (end_sec < start_sec) {
        return 0;
    } else {
        return end_sec - start_sec;
    }
};





struct malloc_list {
    size_t bytes;
    int *address;
};
typedef struct malloc_list malloc_list_t;

malloc_list_t malloc_items[2][NUM_ITEMS];

unsigned free_list[NUM_ITEMS];


int main(int argc, char *argv[])
{
    int i, j, k;
    unsigned tmp;
    unsigned long data_segment_size;
    unsigned long data_segment_free_space;
    struct timespec start_time, end_time;

    srand(0);

    const unsigned chunk_size = 32;
    const unsigned min_chunks = 1;
    const unsigned max_chunks = 2048;
    for (i=0; i < NUM_ITEMS; i++) {
        malloc_items[0][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
        malloc_items[1][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
        free_list[i] = i;
    } //for i

    i = NUM_ITEMS;
    while (i > 1) {
        i--;
        j = rand() % i;
        tmp = free_list[i];
        free_list[i] = free_list[j];
        free_list[j] = tmp;
    } //while


    for (i=0; i < NUM_ITEMS; i++) {
        malloc_items[0][i].address = (int *)MALLOC(malloc_items[0][i].bytes);
    } //for i


    //Start Time
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (i=0; i < NUM_ITERS; i++) {
        unsigned malloc_set = i % 2;
        for (j=0; j < NUM_ITEMS; j+=50) {
            for (k=0; k < 50; k++) {
                unsigned item_to_free = free_list[j+k];
                FREE(malloc_items[malloc_set][item_to_free].address);
            } //for k
            for (k=0; k < 50; k++) {
                malloc_items[1-malloc_set][j+k].address = (int *)MALLOC(malloc_items[1-malloc_set][j+k].bytes);
            } //for k
        } //for j
    } //for i

    //Stop Time
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    data_segment_size = get_data_segment_size();
    data_segment_free_space = get_data_segment_free_space_size();

    double elapsed_ns = calc_time(start_time, end_time);
    printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
    printf("Fragmentation  = %f\n", (float)data_segment_free_space/(float)data_segment_size);

    for (i=0; i < NUM_ITEMS; i++) {
        FREE(malloc_items[0][i].address);
    } //for i

    return 0;
}





//-----------equal
//#include <stdlib.h>
//#include <stdio.h>
//#include <time.h>
//
//
//#define NUM_ITERS    10000
//#define NUM_ITEMS    10000
//#define ALLOC_SIZE   128
//
//
//#define MALLOC(sz) ff_malloc(sz)
//#define FREE(p)    ff_free(p)
//
//#ifdef BF
//#define MALLOC(sz) bf_malloc(sz)
//#define FREE(p)    bf_free(p)
//#endif
//
//
//double calc_time(struct timespec start, struct timespec end) {
//    double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
//    double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;
//
//    if (end_sec < start_sec) {
//        return 0;
//    } else {
//        return end_sec - start_sec;
//    }
//};
//
//
//int main(int argc, char *argv[])
//{
//    int i, j;
//    int *array[NUM_ITEMS];
//    int *spacing_array[NUM_ITEMS];
//    unsigned long data_segment_size;
//    unsigned long data_segment_free_space;
//    struct timespec start_time, end_time;
//
//    if (NUM_ITEMS < 10000) {
//        printf("Error: NUM_ITEMS must be >= 1000\n");
//        return -1;
//    } //if
//
//    for (i=0; i < NUM_ITEMS; i++) {
//        array[i] = (int *)MALLOC(ALLOC_SIZE);
//        spacing_array[i] = (int *)MALLOC(ALLOC_SIZE);
//    } //for i
//
//    for (i=0; i < NUM_ITEMS; i++) {
//        FREE(array[i]);
//    } //for i
//
//    //Start Time
//    clock_gettime(CLOCK_MONOTONIC, &start_time);
//
//
//    for (i=0; i < NUM_ITERS; i++) {
//
//        // printf("i = %d", i);//---
//        for (j=0; j < 1000; j++) {
//            // printf("1st j = %d", j);//---
//            array[j] = (int *)MALLOC(ALLOC_SIZE);
//        } //for j
//
//        for (j=1000; j < NUM_ITEMS; j++) {
//            // printf("2nd j = %d", j);//----
//            array[j] = (int *)MALLOC(ALLOC_SIZE);
//            FREE(array[j-1000]);
//
//            if ((i==NUM_ITERS/2) && (j==NUM_ITEMS/2)) {
//                //Record fragmentation halfway through (try to repsresent steady state)
//                data_segment_size = get_data_segment_size();
//                data_segment_free_space = get_data_segment_free_space_size();
//            } //if
//        } //for j
//
//        for (j=NUM_ITEMS-1000; j < NUM_ITEMS; j++) {
//            // printf("3rd j = %d", j);//-----------
//            FREE(array[j]);
//        } //for j
//    } //for i
//
//    //Stop Time
//    clock_gettime(CLOCK_MONOTONIC, &end_time);
//
//    double elapsed_ns = calc_time(start_time, end_time);
//    printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
//    printf("Fragmentation  = %f\n", (float)data_segment_free_space/(float)data_segment_size);
//
//    for (i=0; i < NUM_ITEMS; i++) {
//        FREE(spacing_array[i]);
//    }
//
//    return 0;
//}










//--------------small
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>
//
//
// #define NUM_ITERS    100 //100
// #define NUM_ITEMS    10000 //10000
//
//
// #define MALLOC(sz) ff_malloc(sz)
// #define FREE(p)    ff_free(p)
//
// #ifdef BF
// #define MALLOC(sz) bf_malloc(sz)
// #define FREE(p)    bf_free(p)
// #endif
//
//
// double calc_time(struct timespec start, struct timespec end) {
//   double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
//   double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;
//
//   if (end_sec < start_sec) {
//     return 0;
//   } else {
//     return end_sec - start_sec;
//   }
// };
//
//
// struct malloc_list {
//   size_t bytes;
//   int *address;
// };
// typedef struct malloc_list malloc_list_t;
//
// malloc_list_t malloc_items[2][NUM_ITEMS];
//
// unsigned free_list[NUM_ITEMS];
//
//
// int main(int argc, char *argv[])
// {
//   int i, j, k;
//   unsigned tmp;
//   unsigned long data_segment_size;
//   unsigned long data_segment_free_space;
//   struct timespec start_time, end_time;
//
//   srand(0);
//
//   const unsigned chunk_size = 32;
//   const unsigned min_chunks = 4;
//   const unsigned max_chunks = 16;
//   for (i=0; i < NUM_ITEMS; i++) {
//     malloc_items[0][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
//     malloc_items[1][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
//     free_list[i] = i;
//   } //for i
//
//   i = NUM_ITEMS;
//   while (i > 1) {
//     i--;
//     j = rand() % i;
//     tmp = free_list[i];
//     free_list[i] = free_list[j];
//     free_list[j] = tmp;
//   } //while
//
//
//   for (i=0; i < NUM_ITEMS; i++) {
//     malloc_items[0][i].address = (int *)MALLOC(malloc_items[0][i].bytes);
//   } //for i
//
//
//   //Start Time
//   clock_gettime(CLOCK_MONOTONIC, &start_time);
//
//   for (i=0; i < NUM_ITERS; i++) {
//     unsigned malloc_set = i % 2;
//     for (j=0; j < NUM_ITEMS; j+=50) {
//       for (k=0; k < 50; k++) {
// 	unsigned item_to_free = free_list[j+k];
// 	FREE(malloc_items[malloc_set][item_to_free].address);
//       } //for k
//       for (k=0; k < 50; k++) {
// 	malloc_items[1-malloc_set][j+k].address = (int *)MALLOC(malloc_items[1-malloc_set][j+k].bytes);
//       } //for k
//     } //for j
//   } //for i
//
//   //Stop Time
//   clock_gettime(CLOCK_MONOTONIC, &end_time);
//
//   data_segment_size = get_data_segment_size();
//   data_segment_free_space = get_data_segment_free_space_size();
//   printf("data_segment_size = %lu, data_segment_free_space = %lu\n", data_segment_size, data_segment_free_space);
//
//   double elapsed_ns = calc_time(start_time, end_time);
//   printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
//   printf("Fragmentation  = %f\n", (float)data_segment_free_space/(float)data_segment_size);
//
//   for (i=0; i < NUM_ITEMS; i++) {
//     FREE(malloc_items[0][i].address);
//   } //for i
//
//   return 0;
// }





//ge---------------------------



#include <stdlib.h>
#include <stdio.h>


//#define MALLOC(sz) ff_malloc(sz)
//#define FREE(p) ff_free(p)
//
//#ifdef BF
//#define MALLOC(sz) bf_malloc(sz)
//#define FREE(p) bf_free(p)
//#endif
//
//int main(int argc, char * argv[]) {
//    const unsigned NUM_ITEMS = 10;
//    int i;
//    int size;
//    int sum = 0;
//    int expected_sum = 0;
//    int * array[NUM_ITEMS];
//
//    size = 4;
//    expected_sum += size * size;
//    array[0] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[0][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[0][i];
//    }  //for i
//
//    size = 16;
//    expected_sum += size * size;
//    array[1] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[1][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[1][i];
//    }  //for i
//
//    size = 8;
//    expected_sum += size * size;
//    array[2] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[2][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[2][i];
//    }  //for i
//
//    size = 32;
//    expected_sum += size * size;
//    array[3] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[3][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[3][i];
//    }  //for i
//
//    FREE(array[0]);
//    FREE(array[2]);
//
//    size = 7;
//    expected_sum += size * size;
//    array[4] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[4][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[4][i];
//    }  //for i
//
//    size = 256;
//    expected_sum += size * size;
//    array[5] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[5][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[5][i];
//    }  //for i
//
//    FREE(array[5]);
//    FREE(array[1]);
//    FREE(array[3]);
//
//    size = 23;
//    expected_sum += size * size;
//    array[6] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[6][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[6][i];
//    }  //for i
//
//    size = 4;
//    expected_sum += size * size;
//    array[7] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[7][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[7][i];
//    }  //for i
//
//    FREE(array[4]);
//
//    size = 10;
//    expected_sum += size * size;
//    array[8] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[8][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[8][i];
//    }  //for i
//
//    size = 32;
//    expected_sum += size * size;
//    array[9] = (int *)MALLOC(size * sizeof(int));
//    for (i = 0; i < size; i++) {
//        array[9][i] = size;
//    }  //for i
//    for (i = 0; i < size; i++) {
//        sum += array[9][i];
//    }  //for i
//
//    FREE(array[6]);
//    FREE(array[7]);
//    FREE(array[8]);
//    FREE(array[9]);
//
//    if (sum == expected_sum) {
//        printf("Calculated expected value of %d\n", sum);
//        printf("Test passed\n");
//    }
//    else {
//        printf("Expected sum=%d but calculated %d\n", expected_sum, sum);
//        printf("Test failed\n");
//    }  //else
//
//    return 0;
//}
