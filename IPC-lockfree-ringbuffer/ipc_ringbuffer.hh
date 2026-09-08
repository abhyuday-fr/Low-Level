#ifndef IPC_RINGBUFFER_H
#define IPC_RINGBUFFER_H

/*
 * for reference:
 * any comment with format: <point-number>
 * contains the note of the corresponding stuff in notes.md file
 * read that file to see how to understand notes
 */

#include <atomic>
#include <cstddef>
#include <cstdint>

constexpr size_t CACHE_LINE_SIZE = 64; // 128 for ARM

// 1
struct alignas(CACHE_LINE_SIZE) SharedRingBufferData {
  std::atomic<size_t> head{0};

  // 2
  alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail{0};

  static constexpr size_t CAPACITY = 1024;
  uint8_t data[CAPACITY];
};

class IPCRingBuffer {
public:
private:
};

#endif
