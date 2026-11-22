#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>

class BumpAllocator {
public:
  explicit BumpAllocator(size_t size) : capacity_(size), offset_(0) {
    memory_ = static_cast<unsigned char *>(std::malloc(size));
  }

  ~BumpAllocator() { std::free(memory_); }

  // disable copy constructor and assignment to prevent double free errors
  BumpAllocator(const BumpAllocator &) = delete;
  BumpAllocator &operator=(const BumpAllocator &) = delete;

  void *alloc(size_t size) {
    if (offset_ + size > capacity_) {
      return nullptr;
    }
    void *ptr = memory_ + offset_;
    offset_ += size;
    return ptr;
  }

  void reset() { offset_ = 0; }

private:
  unsigned char *memory_;
  size_t capacity_;
  size_t offset_;
};
