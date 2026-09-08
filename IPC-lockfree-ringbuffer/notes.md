The cc and hh files have comments with the point numbers to make it easy to read the notes.

# How to read the notes
Let's suppose you are in the header file.
- You see the commment `// 1`
- Open this file side-by-side
- Since you are in header file (.hh), go to the [Header](#header) section of the file
- See the point 1 for the notes regarding the comment on that line.

## Header
1. [Line 17](./ipc_ringbuffer.hh), `struct alignas(64 or 128)` : the strict memory layout that will be mapped into shared memory kept separate from the class to ensure no vtables or class metadata corrupt the shared memory region. Aligning to 64 bytes prevents "false sharing" (cache line bouncing) between the head and tail pointers when accessed by different CPU cores

2. [Line 21](./ipc_ringbuffer.hh), `alignas(64 or 128)` : Forces `tail` to be aligned to a 64-byte boundary. Wasted space to ensure tail starts exactly at the next 64-byte boundary.

**NOTE 1** : we could technically use `alignas(std::hardware_destructive_interference_size)` by include `<new>` header which resolves the CPU's cache line's size for us but there is an issue in that as the producer and consumer are both independent process and if both happen to mismatch the cache line size, we will have silent corrupted data everywhere.
