#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <utility>

template <typename T, std::size_t N> class IntrusiveMutexMemoryPool {
  struct FreeNode {
    FreeNode *next;
  };

  static_assert(sizeof(T) >= sizeof(FreeNode *),
                "T must be large enough to hold a free-list pointer");
  static_assert(alignof(T) >= alignof(FreeNode *),
                "T alignment must satisfy FreeNode* alignment");

  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  std::array<Slot, N> slots{};
  FreeNode *head = nullptr;
  mutable std::mutex mtx;

  void push_free_locked(Slot *slot) noexcept {
    FreeNode *node =
        std::construct_at(reinterpret_cast<FreeNode *>(slot->storage), head);
    head = node;
  }

public:
  IntrusiveMutexMemoryPool() noexcept {
    for (std::size_t i = N; i > 0; --i) {
      push_free_locked(&slots[i - 1]);
    }
  }

  IntrusiveMutexMemoryPool(const IntrusiveMutexMemoryPool &) = delete;
  IntrusiveMutexMemoryPool &
  operator=(const IntrusiveMutexMemoryPool &) = delete;

  template <typename... Args> T *allocate(Args &&...args) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!head) {
      return nullptr;
    }
    FreeNode *node = head;
    head = node->next;
    std::destroy_at(node);
    return std::construct_at(reinterpret_cast<T *>(node),
                             std::forward<Args>(args)...);
  }

  void deallocate(T *obj) noexcept {
    if (!obj) {
      return;
    }
    std::destroy_at(obj);
    auto *slot = reinterpret_cast<Slot *>(obj);
    std::lock_guard<std::mutex> lock(mtx);
    push_free_locked(slot);
  }
};

struct Packet {
  std::size_t seq;
  std::byte payload[32];

  explicit Packet(std::size_t s) : seq(s) {}
};

int main() {
  IntrusiveMutexMemoryPool<Packet, 16> pool;

  Packet *p1 = pool.allocate(101);
  Packet *p2 = pool.allocate(102);

  if (p1 && p2) {
    std::cout << "Packets: " << p1->seq << ", " << p2->seq << "\n";
  }

  // Return p1 first, then p2
  pool.deallocate(p1);
  pool.deallocate(p2);

  // Reuses freed slots without heap allocations
  Packet *p3 = pool.allocate(103);
  if (p3) {
    std::cout << "Reused slot for packet: " << p3->seq << "\n";
    pool.deallocate(p3);
  }
}
