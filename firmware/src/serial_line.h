#pragma once
#include <cstddef>

namespace playground {
enum class LineResult { Incomplete, Ready, Rejected };

// A damaged or overlong command is discarded through its newline. Never execute
// a suffix of the rejected line as a new command.
class SerialLine {
 public:
  LineResult push(char ch) {
    if (ch == '\r') return LineResult::Incomplete;
    if (ch == '\n') {
      const auto result = rejected_ ? LineResult::Rejected : LineResult::Ready;
      buffer_[size_] = 0;
      size_ = 0;
      rejected_ = false;
      return result;
    }
    const auto byte = static_cast<unsigned char>(ch);
    if (byte < 32 || byte > 126 || size_ == sizeof(buffer_) - 1) rejected_ = true;
    if (!rejected_) buffer_[size_++] = ch;
    return LineResult::Incomplete;
  }
  const char* line() const { return buffer_; }
 private:
  char buffer_[64]{};
  size_t size_ = 0;
  bool rejected_ = false;
};
}
