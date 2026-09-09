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
#include <string>

// NOTE 1
constexpr size_t CACHE_LINE_SIZE = 64; // 128 for ARM

// 1
struct alignas(CACHE_LINE_SIZE) SharedRingBufferData {
  std::atomic<size_t> head{0};

  // 2
  alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail{0};

  static constexpr size_t CAPACITY = 1024;

  // 3, NOTE 2
  uint8_t data[CAPACITY]; // fixed size payload
};

class IpcRingBuffer {
public:
  IpcRingBuffer();
  ~IpcRingBuffer();

  // rule-of-three and prevent copying because we manage raw mapped memory
  IpcRingBuffer(const IpcRingBuffer &) = delete;
  IpcRingBuffer &operator=(const IpcRingBuffer &) = delete;

  bool init_as_prodcuer(const std::string &name);
  bool init_as_consumer(const std::string &name);

  bool push(uint8_t value);

private:
  SharedRingBufferData *m_buffer;
  std::string m_name;
  bool is_producer;
  size_t m_mapped_size;

  void cleanup();
};

#endif
