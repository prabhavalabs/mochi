#pragma once
#include "Animator.h"
#include "Command.h"
#include "Renderer.h"

namespace mochi {
inline constexpr float kMaxTriggerSeconds = 86400.f;
// A reusable character instance. Owns appearance and playback, not a display,
// framebuffer, timer, task or menu. All direct calls belong to one owner task.
class Mascot {
 public:
  bool setCharacter(Character character);
  Character character() const { return character_; }
  void setPalette(Palette palette) { palette_ = palette; }
  Palette palette() const { return palette_; }
  // Set the normal activity and cancel any temporary reaction.
  bool setState(State state);
  // Restart a temporary reaction, then return to baseState(). Replaces an
  // existing reaction without stacking it. Duration is (0, 86400] animation seconds.
  bool trigger(State state, float seconds);
  void cancelTrigger();
  bool triggered() const { return triggered_; }
  double triggerRemaining() const { return remaining_; }
  State baseState() const { return base_state_; }
  State state() const { return animator_.state(); }
  const Pose& pose() const { return animator_.pose(); }
  double elapsed() const { return animator_.elapsed(); }
  bool update(float delta_seconds);
  bool dispatch(const Command& command);

  void blink() { animator_.blink(); }
  void setPaused(bool paused) { animator_.setPaused(paused); }
  bool paused() const { return animator_.paused(); }
  bool setSpeed(float speed) { return animator_.setSpeed(speed); }
  float speed() const { return animator_.speed(); }
  bool setSpeechLevel(float level) { return animator_.setSpeechLevel(level); }
  void clearSpeechLevel() { animator_.clearSpeechLevel(); }
  void onStateChange(Animator::StateCallback callback, void* context = nullptr) { animator_.onStateChange(callback, context); }
  void setVisible(bool visible) { visible_ = visible; }
  bool visible() const { return visible_; }
  // Draw relative to this surface's origin. Does not clear, present, or advance
  // time. Hidden instances validate arguments but leave pixels unchanged.
  bool draw(Surface surface, float x, float y, float scale) const;
 private:
  Animator animator_;
  Character character_ = Character::Mochi;
  Palette palette_{};
  State base_state_ = State::Idle;
  double remaining_ = 0;
  bool triggered_ = false, visible_ = true;
};
}
