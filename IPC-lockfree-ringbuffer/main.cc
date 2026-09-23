#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "ipc_ringbuffer.hh"
#include "spin_backoff.hh"

const std::string SHM_NAME = "/spsc_zero_copy_buffer";

void run_producer() {
  IpcRingBuffer ring_buffer;
  if (!ring_buffer.init_as_prodcuer(SHM_NAME)) {
    std::cerr << "Failed to init producer\n";
    return;
  }

  std::cout << "[Producer] ready. Streaming data...\n";

  SpinBackoff backoff;
  uint8_t counter = 0;

  while (true) {
    if (ring_buffer.push(counter)) {
      std::cout << "Sent: " << (int)counter << '\n';
      counter++;

      backoff.reset(); // restore low-latency state on success

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } else {
      backoff.spin();
    }
  }
}

void run_consumer() {
  IpcRingBuffer ring_buffer;
  if (!ring_buffer.init_as_consumer(SHM_NAME)) {
    std::cerr << "Failed to init consumer\n";
    return;
  }

  std::cout << "[Consumer] attatched. Listening for data...\n";

  SpinBackoff backoff;
  uint8_t value;

  while (true) {
    if (ring_buffer.pop(value)) {
      std::cout << "Received: " << (int)value << "\n";

      backoff.reset(); // restore low-latency state on success
    } else {
      backoff.spin();
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " [producer|consumer]\n";
    return EXIT_FAILURE;
  }

  std::string role = argv[1];

  if (role == "producer") {
    run_producer();
  } else if (role == "consumer") {
    run_consumer();
  } else {
    std::cerr << "Invalid argument\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
