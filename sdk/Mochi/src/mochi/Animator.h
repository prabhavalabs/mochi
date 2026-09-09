#pragma once
#include "Motion.h"

namespace mochi {
// Calls are synchronous and belong on the application's animation/UI task.
// Each instance owns its playback state and performs no heap allocation.
class Animator {
 public:
  using StateCallback = void (*)(State previous, State current, void* context);

  Animator();
  bool setState(State state);
  // Replay the current state without a state-change callback. Keeps playback
  // settings and blends from the current pose; a paused replay snaps immediately.
  void restart();
  void next();
  void previous();
  void blink();
  bool setSpeed(float multiplier); // Valid range: 0.1..4.0, finite.
  void setPaused(bool paused) { paused_ = paused; }
  bool setSpeechLevel(float level); // 0..1 mouth opening in Speaking; no audio I/O.
  void clearSpeechLevel() { speech_level_ = -1; }
  void onStateChange(StateCallback callback, void* context = nullptr);
  // Elapsed wall time in seconds. Invalid/nonpositive deltas are ignored.
  // At most 100 ms is consumed per call to avoid jumping after a long stall.
  bool update(float delta_seconds);

  State state() const { return state_; }
  const Pose& pose() const { return pose_; }
  bool paused() const { return paused_; }
  float speed() const { return speed_; }
  double elapsed() const { return elapsed_; }

 private:
  Pose targetPose() const;
  State state_ = State::Idle;
  Pose pose_{};
  double elapsed_ = 0;
  float blink_age_ = -1, speed_ = 1, speech_level_ = -1;
  bool paused_ = false;
  StateCallback callback_ = nullptr;
  void* context_ = nullptr;
};
}
