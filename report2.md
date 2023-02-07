# Report of Project 2-Thread-Safe Malloc 

### Yuguo Wang  
### NetID: yw540



## 1. Implement

These two thread-safe versions of `malloc()` and `free()` are based on best-fit from project 1. We implemented a Double LinkedList to store free block. When malloc, if there is no headBlock, we need allocate a space for the headBlock. Then, we search the linked list. If there is a block founded whose datasize is bigger than requested size, we could allocate space out from free space and do "split" and "remove" , otherwise, we need to call `sbrk()` to ask for more space, which is to add the new block behind the linkedlist.

### Lock Version

For lock version, as the linked list can be accessed by all threads, any operation of the linked list could be influenced by other threads. Therefore, for function `malloc()` and `free()`, we need to lock at the beginning of them, and unlock just before `return`, to avoid the overlapped regions. In the lock version, no concurency happens.


### Nolock Version

For nolock version, thread-local storage is used. As the free linkedlist is shared by all threads, we defined our `headBlock_unlock` node by using `__thread block *`. In this way, each thread will have their own headBlock and there will be no race conditions. And as `sbrk()` function is not thread-safe, so we need to lock immediately before `sbrk()` and unlock immediately after `sbrk()`. In this nolock version, concurency is allowed. More than one thread might happen at the same time, which help save some time.



## 2. Measurement

|                | Time   | Data Segment Size |
| -------------- | ------ | ----------------- |
| Lock Version   | 0.12 s | 43117336 bytes    |
| Nolock Version | 0.04 s | 42135816 bytes    |

#### Lock Version spends more time than Nolock Version

As is mentioned above, all threads need to go through one by one in lock version. However, in nolock version, all threads could happen togethe, which save a lot of time.

#### The space between the two version did not make difference

I tried the measurement several times and found that there is not a obvious trend about the data segment size between the two versions. For the two versions, the free list always exist no matter how many threads are in the process at the same time.
