#include "ipc_ringbuffer.hh"
#include <cstdint>
#include <string>

IpcRingBuffer::IpcRingBuffer()
    : m_buffer(nullptr), m_is_producer(false),
      m_mapped_size(sizeof(SharedRingBufferData)) {}

IpcRingBuffer::~IpcRingBuffer() { cleanup(); }

bool IpcRingBuffer::init_as_prodcuer(const std::string &name) {}

bool IpcRingBuffer::init_as_consumer(const std::string &name) { return false; }

bool IpcRingBuffer::push(uint8_t value) { return false; }

void IpcRingBuffer::cleanup() {}
