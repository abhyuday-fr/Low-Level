#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

template <typename T, std::size_t N> class MutexMemoryPool {
  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  struct Node {
    Slot *slot = nullptr;
    Node *next = nullptr;
  };

  std::array<Slot, N> slots{};
  std::array<Node, N> nodes{};
  Node *head = nullptr;
  mutable std::mutex mtx;

  void push_free_locked(Slot *slot, Node *node) noexcept {
    node->slot = slot;
    node->next = head;
    head = node;
  }

  Node *node_for(const Slot *slot) noexcept {
    const auto idx = static_cast<std::size_t>(slot - slots.data());
    return &nodes[idx];
  }

public:
  MutexMemoryPool() noexcept {
    for (std::size_t i = N; i > 0; --i) {
      push_free_locked(&slots[i - 1], &nodes[i - 1]);
    }
  }

  MutexMemoryPool(const MutexMemoryPool &) = delete;
  MutexMemoryPool &operator=(const MutexMemoryPool &) = delete;

  template <typename... Args> T *allocate(Args &&...args) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!head) {
      return nullptr;
    }
    Node *node = head;
    head = node->next;
    node->next = nullptr;
    return std::construct_at(reinterpret_cast<T *>(node->slot->storage),
                             std::forward<Args>(args)...);
  }

  void deallocate(T *obj) noexcept {
    if (!obj) {
      return;
    }
    std::destroy_at(obj);

    auto *slot = reinterpret_cast<Slot *>(reinterpret_cast<std::byte *>(obj) -
                                          offsetof(Slot, storage));

    std::lock_guard<std::mutex> lock(mtx);
    push_free_locked(slot, node_for(slot));
  }
};

struct Task {
  int id;
  std::string label;

  Task(int i, std::string l) : id(i), label(std::move(l)) {}
};

int main() {
  MutexMemoryPool<Task, 64> pool;
  std::vector<std::thread> workers;
  for (int t = 0; t < 4; ++t) {
    workers.emplace_back([&, t]() {
      Task *item = pool.allocate(t * 10 + 1, "worker-task");
      if (!item) {
        std::cout << "Pool exhausted on thread " << t << "\n";
        return;
      }

      std::cout << "Allocated Task #" << item->id << " (" << item->label
                << ")\n";
      pool.deallocate(item);
    });
  }

  for (auto &w : workers) {
    w.join();
  }
}
