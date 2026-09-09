#include "mochi/Animator.h"
#include <algorithm>
#include <cmath>

namespace mochi {
Animator::Animator() : pose_(sample_pose(State::Idle, 0)) {}
bool Animator::setState(State state) {
  if (static_cast<unsigned>(state) >= kStateCount) return false;
  if (state == state_) return true;
  const State previous = state_;
  state_ = state;
  elapsed_ = 0;
  blink_age_ = -1;
  // A state picker can preview a different expression while playback is paused.
  if (paused_) pose_ = targetPose();
  if (callback_) callback_(previous, state_, context_);
  return true;
}
void Animator::next() { setState(static_cast<State>((static_cast<int>(state_) + 1) % kStateCount)); }
void Animator::previous() { setState(static_cast<State>((static_cast<int>(state_) + kStateCount - 1) % kStateCount)); }
void Animator::restart() {
  elapsed_ = 0;
  blink_age_ = -1;
  if (paused_) pose_ = targetPose();
}
void Animator::blink() { blink_age_ = 0; }
bool Animator::setSpeed(float multiplier) {
  if (!std::isfinite(multiplier) || multiplier < .1f || multiplier > 4) return false;
  speed_ = multiplier;
  return true;
}
bool Animator::setSpeechLevel(float level) {
  if (!std::isfinite(level) || level < 0 || level > 1) return false;
  speech_level_ = level;
  return true;
}
void Animator::onStateChange(StateCallback callback, void* context) {
  callback_ = callback;
  context_ = context;
}
Pose Animator::targetPose() const {
  Pose target = sample_pose(state_, elapsed_, blink_age_ < 0 ? 0 : blink_at(blink_age_, .12f));
  if (state_ == State::Speaking && speech_level_ >= 0) target.mouth_open = speech_level_;
  return target;
}
bool Animator::update(float dt) {
  if (paused_ || !std::isfinite(dt) || dt <= 0) return false;
  dt = std::min(dt, .1f) * speed_;
  elapsed_ += dt;
  if (blink_age_ >= 0) {
    blink_age_ += dt;
    if (blink_age_ > .3f) blink_age_ = -1;
  }
  blend_pose(pose_, targetPose(), dt);
  return true;
}
}
