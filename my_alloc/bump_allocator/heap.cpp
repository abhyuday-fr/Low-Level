#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

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

struct Person { // will allocate this struct
  int id;
  char name[32];
};

int main() {
  BumpAllocator memory(1024 * 1024); // the big chunk in heap

  // allocating just a number (variable)
  int *num =
      (int *)memory.alloc(sizeof(int)); // see the similarity with malloc(3)
  *num = 42; // storing data into allocated int variable

  // allocating the struct
  Person *user = (Person *)memory.alloc(sizeof(Person));
  user->id = 101;
  std::strcpy(user->name, "ABCxyz");

  // reading data
  std::cout << "int variable: " << *num << '\n';
  std::cout << "user id: " << user->id << ", name: " << user->name << '\n';

  memory.reset();
}
