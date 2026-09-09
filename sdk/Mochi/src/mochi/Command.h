#pragma once
#include "Character.h"
#include "Motion.h"
#include <type_traits>

namespace mochi {
enum class CommandType : uint8_t {
  SetState, Trigger, CancelTrigger, SetCharacter, Blink, SetPaused, SetSpeed,
  SetSpeechLevel, ClearSpeechLevel, SetVisible
};
// Value-only payload: copy it through your application's task/event queue.
// The receiving UI task calls Mascot::dispatch(). No object pointers are retained.
struct Command {
  CommandType type = CommandType::Blink;
  State state = State::Idle;
  Character character = Character::Mochi;
  float value = 0;
  bool flag = false;

  static constexpr Command setState(State state) { return {CommandType::SetState, state}; }
  static constexpr Command trigger(State state, float seconds) { return {CommandType::Trigger, state, Character::Mochi, seconds}; }
  static constexpr Command cancelTrigger() { return {CommandType::CancelTrigger}; }
  static constexpr Command setCharacter(Character character) { return {CommandType::SetCharacter, State::Idle, character}; }
  static constexpr Command blink() { return {}; }
  static constexpr Command setPaused(bool paused) { return {CommandType::SetPaused, State::Idle, Character::Mochi, 0, paused}; }
  static constexpr Command setSpeed(float speed) { return {CommandType::SetSpeed, State::Idle, Character::Mochi, speed}; }
  static constexpr Command setSpeechLevel(float level) { return {CommandType::SetSpeechLevel, State::Idle, Character::Mochi, level}; }
  static constexpr Command clearSpeechLevel() { return {CommandType::ClearSpeechLevel}; }
  static constexpr Command setVisible(bool visible) { return {CommandType::SetVisible, State::Idle, Character::Mochi, 0, visible}; }
};
static_assert(std::is_trivially_copyable<Command>::value, "Commands must support byte-copy task queues");
}
