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

3. [Line 28](./ipc_ringbuffer.hh) `uint8_t data[1024]` : The fixed size payload uses 8 bits integers as it is the simplest way to demonstrate the lock-free pointer math in example.

**NOTE 2** : to use the variable length data. Using a generic byte array turns our ring buffer into a universal transport layer, very similar to a raw TCP socket.We can use a struct like

```cpp
struct alignas(N) LogMessage {
    uint64_t timestamp;
    uint32_t severity;
    char text[116]; // Padded so the whole struct is exactly 128 bytes
};
```
We padded the LogMessage to 128 bytes because we are taking a long message here (can use any multiple of 64, do padding accordingly).
then use the `LogMessage` in the `SharedRingBuffer`
```cpp
struct alignas(N) SharedRingBufferData {
    std::atomic<size_t> head{0};
    alignas(N) std::atomic<size_t> tail{0};
    
    static constexpr size_t CAPACITY = 1024;
    
    // The buffer now holds exactly 1024 complete LogMessages
    LogMessage data[CAPACITY]; 
};
```
**The Virtual Memory Trap** : If you change the payload to a custom struct, there is one absolute rule you must follow for IPC: The struct must be Trivially Copyable (Plain Old Data).

It cannot contain std::string, std::vector, or any raw pointers.

Because Process A and Process B map the shared memory into completely different virtual address spaces, a pointer created by Process A points to a specific virtual address. If you send that pointer through the ring buffer, Process B will read it and try to access that same virtual address in its own memory space which will either contain garbage data or trigger an instant segmentation fault.

All data sent through an IPC ring buffer must be entirely self-contained within fixed-size arrays inside the struct.
