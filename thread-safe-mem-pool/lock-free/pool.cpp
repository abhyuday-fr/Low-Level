#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <thread>
#include <utility>
#include <vector>

template <typename T, std::size_t N> class AtomicIntrusiveMemoryPool {
  static_assert(N < UINT32_MAX, "Pool size must fit in uint32_t index space");

  struct FreeNode {
    std::uint32_t next;
  };

  static_assert(sizeof(T) >= sizeof(FreeNode),
                "T must be large enough to store next index");
  static_assert(alignof(T) >= alignof(FreeNode),
                "T alignment must satisfy FreeNode alignment");

  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  static constexpr std::uint32_t kNull = UINT32_MAX;

  static constexpr std::uint64_t pack(std::uint32_t tag,
                                      std::uint32_t idx) noexcept {
    return (static_cast<std::uint64_t>(tag) << 32) | idx;
  }

  static constexpr std::uint32_t unpack_tag(std::uint64_t state) noexcept {
    return static_cast<std::uint32_t>(state >> 32);
  }

  static constexpr std::uint32_t unpack_idx(std::uint64_t state) noexcept {
    return static_cast<std::uint32_t>(state & 0xFFFFFFFFULL);
  }

  std::array<Slot, N> slots{};
  std::atomic<std::uint64_t> head{pack(0, N == 0 ? kNull : 0)};

  FreeNode *node_at(std::uint32_t idx) noexcept {
    return std::launder(reinterpret_cast<FreeNode *>(slots[idx].storage));
  }

  std::uint32_t index_of(const Slot *slot) const noexcept {
    return static_cast<std::uint32_t>(slot - slots.data());
  }

public:
  AtomicIntrusiveMemoryPool() noexcept {
    for (std::uint32_t i = 0; i < N; ++i) {
      std::uint32_t next_idx = (i + 1 < N) ? (i + 1) : kNull;
      std::construct_at(reinterpret_cast<FreeNode *>(slots[i].storage),
                        next_idx);
    }
  }

  AtomicIntrusiveMemoryPool(const AtomicIntrusiveMemoryPool &) = delete;
  AtomicIntrusiveMemoryPool &
  operator=(const AtomicIntrusiveMemoryPool &) = delete;

  template <typename... Args> T *allocate(Args &&...args) {
    std::uint64_t current = head.load(std::memory_order_acquire);
    while (true) {
      std::uint32_t idx = unpack_idx(current);
      if (idx == kNull) {
        return nullptr;
      }

      std::uint32_t next_idx = node_at(idx)->next;
      std::uint32_t next_tag = unpack_tag(current) + 1;
      std::uint64_t desired = pack(next_tag, next_idx);

      if (head.compare_exchange_weak(current, desired,
                                     std::memory_order_acq_rel,
                                     std::memory_order_acquire)) {
        std::destroy_at(node_at(idx));
        return std::construct_at(reinterpret_cast<T *>(slots[idx].storage),
                                 std::forward<Args>(args)...);
      }
    }
  }

  void deallocate(T *obj) noexcept {
    if (!obj) {
      return;
    }
    std::destroy_at(obj);

    auto *slot = reinterpret_cast<Slot *>(obj);
    std::uint32_t idx = index_of(slot);
    FreeNode *node =
        std::construct_at(reinterpret_cast<FreeNode *>(slot->storage), kNull);

    std::uint64_t current = head.load(std::memory_order_relaxed);
    while (true) {
      std::uint32_t cur_idx = unpack_idx(current);
      std::uint32_t next_tag = unpack_tag(current) + 1;
      node->next = cur_idx;
      std::uint64_t desired = pack(next_tag, idx);

      if (head.compare_exchange_weak(current, desired,
                                     std::memory_order_release,
                                     std::memory_order_relaxed)) {
        return;
      }
    }
  }
};

struct Event {
  std::uint64_t timestamp;
  std::uint64_t code;

  Event(std::uint64_t ts, std::uint64_t c) : timestamp(ts), code(c) {}
};

int main() {
  AtomicIntrusiveMemoryPool<Event, 128> pool;
  std::atomic<int> processed{0};

  std::vector<std::thread> threads;
  for (int t = 0; t < 4; ++t) {
    threads.emplace_back([&, t]() {
      for (int i = 0; i < 20; ++i) {
        Event *ev = pool.allocate(static_cast<std::uint64_t>(i),
                                  static_cast<std::uint64_t>(t));
        if (ev) {
          processed.fetch_add(1, std::memory_order_relaxed);
          pool.deallocate(ev);
        }
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  std::cout << "Total events processed: " << processed.load() << "\n";
}
