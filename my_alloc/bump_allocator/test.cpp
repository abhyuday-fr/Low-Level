#include "allocator.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#define NUM_OF_PERSONS 5'000'000

struct Person {
  int id;
  char name[32];
};

static volatile char sink = 0;

int main() {
  size_t total_bytes = NUM_OF_PERSONS * sizeof(Person);
  BumpAllocator memory(total_bytes);

  // PRE-FAULT / WARM-UP (Forces OS page faults BEFORE start1)
  void *dummy = memory.alloc(total_bytes);
  std::memset(dummy, 0, total_bytes); // Touch all 180 MB of pages
  memory.reset();                     // Reset offset back to 0

  // -------------------------------------------------------------
  // TEST 1: Bump Allocator
  // -------------------------------------------------------------
  auto start1 = std::chrono::steady_clock::now();

  for (size_t i = 0; i < NUM_OF_PERSONS; i++) {
    Person *user = (Person *)memory.alloc(sizeof(Person));
    user->id = static_cast<int>(i);
    user->name[0] = 'A';
    sink = user->name[0];
  }

  memory.reset();
  auto end1 = std::chrono::steady_clock::now();

  // TEST 2: Naive malloc (forces 5,000,000 distinct heap allocations)
  std::vector<Person *> ptrs(NUM_OF_PERSONS);

  auto start2 = std::chrono::steady_clock::now();

  for (size_t i = 0; i < NUM_OF_PERSONS; i++) {
    ptrs[i] = (Person *)malloc(sizeof(Person));
    ptrs[i]->id = static_cast<int>(i);
    ptrs[i]->name[0] = 'A';
    sink = ptrs[i]->name[0];
  }

  for (size_t i = 0; i < NUM_OF_PERSONS; i++) {
    free(ptrs[i]);
  }

  auto end2 = std::chrono::steady_clock::now();

  auto duration1 =
      std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
  auto duration2 =
      std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

  std::cout << "bump allocator: " << duration1.count() << " ms\n";
  std::cout << "naive malloc   : " << duration2.count() << " ms\n";
}
