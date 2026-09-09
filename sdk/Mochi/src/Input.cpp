#include "mochi/Input.h"
#include <algorithm>
#include <cstdlib>

namespace mochi {
Gesture GestureRecognizer::update(TouchReport report, int16_t x, int16_t y, uint32_t now) {
  if (report == TouchReport::Pressed) {
    if (!pressed_) { start_x_ = x; start_y_ = y; max_dx_ = max_dy_ = 0; }
    pressed_ = true; x_ = x; y_ = y; last_report_ = now;
    max_dx_ = std::max(max_dx_, std::abs(int32_t(x_) - start_x_));
    max_dy_ = std::max(max_dy_, std::abs(int32_t(y_) - start_y_));
    return {};
  }
  if (!pressed_) return {};
  if (now - last_report_ > config_.release_timeout_ms) {
    pressed_ = false;
    return {GestureType::Cancelled, x_, y_};
  }
  if (report != TouchReport::Released) return {};
  pressed_ = false;
  const int32_t dx = int32_t(x_) - start_x_, dy = int32_t(y_) - start_y_;
  if (std::abs(dx) > config_.swipe_distance && std::abs(dx) > std::abs(dy))
    return {dx < 0 ? GestureType::SwipeLeft : GestureType::SwipeRight, x_, y_};
  if (max_dx_ < config_.tap_slop && max_dy_ < config_.tap_slop) return {GestureType::Tap, x_, y_};
  return {GestureType::Cancelled, x_, y_};
}
}
