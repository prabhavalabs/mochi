#pragma once

#include <cstdint>

namespace mochi {
enum class State : uint8_t {
  Idle, Blinking, Happy, Listening, Thinking, Speaking, Sleeping, Surprised,
  Sad, Waving, Count
};
constexpr int kStateCount = static_cast<int>(State::Count);
const char* state_name(State state);
const char* state_detail(State state);

struct Pose {
  float y = 0, sx = 1, sy = 1, tilt = 0, gaze_x = 0, gaze_y = 0;
  float eye_l = 1, eye_r = 1, happy_eyes = 0;
  float smile = 1, mouth_open = 0, mouth_round = 0, mouth_x = 0;
  float brow_l = 0, brow_r = 0, brow_sad = 0;
  float left_x = -153, left_y = 35, left_angle = -.2f;
  float right_x = 153, right_y = 35, right_angle = .2f, cheeks = 1;
};
Pose sample_pose(State state, double seconds, float blink = 0);
float blink_at(float seconds, float center);
void blend_pose(Pose& current, const Pose& target, float seconds);
}
