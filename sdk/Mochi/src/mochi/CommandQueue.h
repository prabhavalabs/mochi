#pragma once
#include "Command.h"
#include <array>
#include <atomic>
#include <cstddef>
#include <limits>

namespace mochi {
// Optional bounded queue for EXACTLY one producer and one consumer. For multiple
// producers use an RTOS queue or serialize producers outside this class.
// Queue lifetime must span both tasks. Commands are rejected, never overwritten,
// when full; the producer decides whether to retry, coalesce or drop the event.
template<size_t Capacity>
class CommandQueue {
  static_assert(Capacity > 0 && Capacity < std::numeric_limits<size_t>::max(), "Queue capacity must be positive and representable");
 public:
  static constexpr size_t capacity = Capacity;
  static constexpr bool always_lock_free = std::atomic<size_t>::is_always_lock_free;
  bool push(const Command& command) {
    const size_t write = write_.load(std::memory_order_relaxed);
    const size_t next = advance(write);
    if (next == read_.load(std::memory_order_acquire)) return false;
    commands_[write] = command;
    write_.store(next, std::memory_order_release);
    return true;
  }
  bool pop(Command& command) {
    const size_t read = read_.load(std::memory_order_relaxed);
    if (read == write_.load(std::memory_order_acquire)) return false;
    command = commands_[read];
    read_.store(advance(read), std::memory_order_release);
    return true;
  }
 private:
  static constexpr size_t advance(size_t index) { return index == Capacity ? 0 : index + 1; }
  std::array<Command, Capacity + 1> commands_{};
  std::atomic<size_t> read_{0}, write_{0};
};
}
