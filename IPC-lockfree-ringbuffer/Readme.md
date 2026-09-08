# IPC Lock-free Ring Buffer
Designing a lock-free ring buffer inside shared memory relies heavily on atomic operations and strict memory ordering constraints.
The most practical and efficient approach is the Single-Producer, Single-Consumer (SPSC) architecture.

# A note for the ARM architecture users
in the [header](./ipc_ringbuffer.hh) around line 17 set `CACHE_LINE_SIZE` to `128`

## Notes of this project
view [notes](notes.md) because adding comments is very tedious
