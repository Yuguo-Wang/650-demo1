##### Report of Project 1 (Malloc Library)

##### Yuguo(Harry) Wang      NetID:yw540



##### Data Structure

Implemented a Double LinkedList to store free block.

Each block in the free list contains a metadata and the space for actual data space(block->blockSize). 

The way of manage the Linkedlist is to let the block points to the start of metadata.

Block contains

* `block * next`: A pointer to next block (start of metadata)
* `block * prev`: A pointer to prev block (start of metadata)
* `size_t blockSize`: describe how much size the space is.


##### Allocate

Call `sbrk` in liunx environment

##### Malloc

When malloc, if there is no headBlock, we need allocate a space for the headBlock. Then, we search the linked list. If there is a block founded whose datasize is bigger than requested size, we could allocate space out from free space and do "split" and "remove" , otherwise, we need to call `sbrk()` to ask for more space, which is to add the new block behind the linkedlist.

##### First Fit

Stop searching as soon as we find a Block whose datasize is bigger than size that users give, if no block satisfies, allocate a new space.

##### Best Fit

Search the whole linked list, choose the Block whose datasize is the smallest among Blocks whose datasize is bigger than size, if no block satisfies, allocate a new space. As soon as we search a block whose datasize equals size, we could use this block because this must be the smallest node. That is ver important, which means we stop searching the block with same size, and return the first block whose blocksize is the same as the size that users give.

##### Split and Remove

If we find a Node whose datasize is bigger than requested size, we need to edit the linked list to show the change. There are two situations here:

* `block->blockSize <= sizeof(block) + size`

When datasize is smaller than the size of metadata and requested size, we need to remove the Node, because after allocating the space out, the left space will not be enough to store metadata, no need to say actual data.

* `block->blockSize > sizeof(block) + size`

When datasize is larger than the size of metadata and requested size, we need to spilt this block (block->blockSize -= sizeof(block) + size) because after allocating out the space, the left space will still be large enough to store metadata and actual data. And then, we need to remove the splitted space.

The whole process is to split the block first, which is similar to adding a new block into the linkedlist, and then we remove the block which is needless. 


##### Free and merge

This process is similar to adding a block in a linkedlist in an ascending order by point to the start of metadata. After adding the block, we have to consider if there is any blocks that can be merged, which means the former block's end is equal to current block's start. And merge with right block is the same. And it's better to merge right before merge left, which will not change the point to the start of the metadata.

There is no different between first fit free and best fit free.


##### `User` and `Manage` point *
For malloc, we have to make sure that, the return point should be the point for user, which is `(void*)ptr + sizeof(block)`；For free, the argument point comes from user, and we need to manage the point by `(void*)ptr - sizeof(block)`




##### Study of Performance Policy


##### data_segment_size

A global variable called `data_segment_size` and set it to 0 at beginning. Add sizeof(block) + size to it everytime calling `sbrk()`. 

##### data_segment_free_space_size

A global data called `data_segment_free_size` and set it to 0 at beginning. At last, search the freelist and add sizeof(block) + currBlock->blockSize to it from every block.




##### Analysis of Performance

##### Results

#NUM_ITERs 100 for small, 10000 for equal, 50 for large

#NUM_ITEMs 10000 for all sizes

|            | First Fit(Time/Fragmentation) | Best Fit(Time/Fragmentation) |
| ---------- | ----------------------------- | ---------------------------- |
| Equal_size | 8.63s / 0.45                  | 9.43s / 0.45                 |
| Small_size | 13.03s / 0.073                | 3.19s / 0.027                |
| Large_size | 84.60s / 0.093                | 123.16s / 0.041              |

##### Equal size

There should be no difference between first fit and best fit for equal_size. Since all blocks are in equal size, the best fit should also be the first fit. Because all free spaces are in same size, the first block in the linked list will be enough for the requested space, we do not need to search through the whole linked list, therefore, the time for equal_size is quite short.

##### Fragmentation is better for best fit

There is a formula that `Fragmentation=free space/total space`, so the smaller the fragmentation, the better use of space. There are more larger spaces saved for data to use during the malloc process, because best fit always searches the minimum space that could fit requested size. However, first fit always give the first fit space out, which tends to split large space into small pieces, which causes a waste of space, because these small spaces may not be used later.

##### Time difference between first fit and best fit 

Overall, large size will take more time than small size, which is easy to find.

For larger size, best fit take longer time but it is opposite for small size. This is because best fit usually search the whole linked list, whereas since best fit takes good use of spacem, which saves time in total, to some degree. 

In contrast, first fit saves time at a single search, however it will split the space and causes that no space could be used in the linked list at the end. Therefore, it will result in a situation that we have to search the whole linked list every time and then ask for allocating new space.

For the equal size, which is explained above, it's much more easier to find the block in same blockSize. And that is why we find that it will take shorter time.
