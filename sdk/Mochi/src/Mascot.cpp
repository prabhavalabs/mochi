#include "mochi/Mascot.h"
#include <algorithm>
#include <cmath>

namespace mochi {
bool Mascot::setCharacter(Character character) {
  if (static_cast<unsigned>(character) >= kCharacterCount) return false;
  if (character != character_) {
    character_ = character;
    palette_ = default_palette(character);
  }
  return true;
}
bool Mascot::setState(State state) {
  if (static_cast<unsigned>(state) >= kStateCount) return false;
  base_state_ = state;
  triggered_ = false;
  remaining_ = 0;
  return animator_.setState(state);
}
bool Mascot::trigger(State state, float seconds) {
  if (static_cast<unsigned>(state) >= kStateCount || !std::isfinite(seconds) ||
      seconds <= 0 || seconds > kMaxTriggerSeconds) return false;
  triggered_ = true;
  remaining_ = seconds;
  if (state == animator_.state()) animator_.restart();
  else animator_.setState(state);
  return true;
}
void Mascot::cancelTrigger() {
  if (!triggered_) return;
  triggered_ = false;
  remaining_ = 0;
  animator_.setState(base_state_);
}
bool Mascot::update(float dt) {
  const double before = animator_.elapsed();
  if (!animator_.update(dt)) return false;
  if (triggered_) {
    remaining_ = std::max(0.0, remaining_ - (animator_.elapsed() - before));
    if (remaining_ == 0) cancelTrigger();
  }
  return true;
}
bool Mascot::dispatch(const Command& command) {
  switch (command.type) {
    case CommandType::SetState: return setState(command.state);
    case CommandType::Trigger: return trigger(command.state, command.value);
    case CommandType::CancelTrigger: cancelTrigger(); return true;
    case CommandType::SetCharacter: return setCharacter(command.character);
    case CommandType::Blink: blink(); return true;
    case CommandType::SetPaused: setPaused(command.flag); return true;
    case CommandType::SetSpeed: return setSpeed(command.value);
    case CommandType::SetSpeechLevel: return setSpeechLevel(command.value);
    case CommandType::ClearSpeechLevel: clearSpeechLevel(); return true;
    case CommandType::SetVisible: setVisible(command.flag); return true;
    default: return false;
  }
}
bool Mascot::draw(Surface surface, float x, float y, float scale) const {
  if (!valid_surface(surface) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(scale) ||
      std::fabs(x) > 1000000 || std::fabs(y) > 1000000 || scale <= 0 || scale > 100) return false;
  if (!visible_) return true;
  Renderer renderer(surface);
  renderer.setCharacter(character_);
  renderer.setPalette(palette_);
  return renderer.draw(animator_.pose(), x, y, scale);
}
}
