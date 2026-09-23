#ifndef SPIN_BACKOFF_H
#define SPIN_BACKOFF_H

/*
 * refer Performance_Bottleneck.md for the context of this file and its
 * functionality
 */

#include <chrono>
#include <thread>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) ||             \
    defined(_M_IX86)
#include <immintrin.h>
#define CPU_PAUSE() _mm_pause()
#elif defined(__aarch64__)
#define CPU_PAUSE() __asm__ volatile("yield" ::: "memory")
#else
#define CPU_PAUSE()
#endif

class SpinBackoff {
  int attempts = 0;

public:
  void spin() {
    attempts++;

    if (attempts < 10) {
      CPU_PAUSE();
    } else if (attempts < 20) {
      std::this_thread::yield();
    } else {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
  }

  void reset() { attempts = 0; }
};

#endif
