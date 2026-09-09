#pragma once
#include <cstdint>

namespace mochi {
// Appearance is independent of animation state and playback.
enum class Character : uint8_t { Mochi, Sprout, Peach, Nimbus, Count };
inline constexpr int kCharacterCount = static_cast<int>(Character::Count);
const char* character_name(Character character);
const char* character_detail(Character character);
}
