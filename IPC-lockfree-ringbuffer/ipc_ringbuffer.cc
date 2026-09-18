#include "ipc_ringbuffer.hh"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <sys/mman.h> // for mmap(2)
#include <sys/stat.h>
#include <unistd.h>

/*
 * for reference:
 * any comment with format: <point-number>
 * contains the note of the corresponding stuff in notes.md file
 * read that file to see how to understand notes
 */

IpcRingBuffer::IpcRingBuffer()
    : m_buffer(nullptr), m_is_producer(false),
      m_mapped_size(sizeof(SharedRingBufferData)) {}

IpcRingBuffer::~IpcRingBuffer() { cleanup(); }

bool IpcRingBuffer::init_as_prodcuer(const std::string &name) {
  m_name = name;
  m_is_producer = true;

  // 1
  shm_unlink(m_name.c_str());

  // 2
  int fd = shm_open(m_name.c_str(), O_CREAT | O_RDWR, 0666);
  if (fd == -1) {
    perror("shm_open failed");
    return false;
  }

  // 3
  if (ftruncate(fd, m_mapped_size) == -1) {
    perror("ftruncate failed");
    close(fd);
    return false;
  }

  void *ptr = mmap(0, m_mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  // file descriptor no longer needed after mapping

  if (ptr == MAP_FAILED) {
    perror("mmap failed");
    return false;
  }

  m_buffer = new (ptr) SharedRingBufferData();

  return true;
}

bool IpcRingBuffer::init_as_consumer(const std::string &name) {
  m_name = name;
  m_is_producer = false;

  // 4
  int fd = shm_open(m_name.c_str(), O_RDWR, 0666);
  if (fd < 0) {
    perror("shm_open failed. Is the producer running?");
    return false;
  }

  void *ptr = mmap(0, m_mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (ptr == MAP_FAILED) {
    perror("mmap failed");
    return false;
  }

  m_buffer = static_cast<SharedRingBufferData *>(ptr);
  return true;
}

bool IpcRingBuffer::push(uint8_t value) {

  if (!m_buffer) {
    return false;
  }

  // 5 a)
  size_t current_head = m_buffer->head.load(std::memory_order_relaxed);

  // 5 b)
  size_t current_tail = m_buffer->tail.load(std::memory_order_acquire);

  size_t next_head = (current_head + 1) % SharedRingBufferData::CAPACITY;
  if (next_head == current_tail) {
    return false; // full
  }

  m_buffer->data[current_head] = value;

  // 5 c)
  m_buffer->head.store(next_head, std::memory_order_release);
  return true;
}

bool IpcRingBuffer::pop(uint8_t &out_value) {
  if (!m_buffer) {
    return false;
  }

  size_t current_tail = m_buffer->tail.load(std::memory_order_relaxed);
  size_t current_head = m_buffer->head.load(std::memory_order_acquire);

  if (current_head == current_tail) {
    return false; // empty
  }

  out_value = m_buffer->data[current_tail];
  size_t next_tail = (current_tail + 1) % SharedRingBufferData::CAPACITY;
  m_buffer->tail.store(next_tail, std::memory_order_release);
  return true;
}

void IpcRingBuffer::cleanup() {
  if (m_buffer) {
    munmap(m_buffer, m_mapped_size);
    m_buffer = nullptr;
  }

  // only the producer is responsible for destroying the shared memory object
  if (m_is_producer && !m_name.empty()) {
    shm_unlink(m_name.c_str());
  }
}
