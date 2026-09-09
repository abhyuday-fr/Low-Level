#include "ipc_ringbuffer.hh"
#include <cstdint>
#include <string>

IpcRingBuffer::IpcRingBuffer() {}

IpcRingBuffer::~IpcRingBuffer() {}

bool IpcRingBuffer::init_as_prodcuer(const std::string &name) { return false; }

bool IpcRingBuffer::init_as_consumer(const std::string &name) { return false; }

bool IpcRingBuffer::push(uint8_t value) { return false; }

void IpcRingBuffer::cleanup() {}
