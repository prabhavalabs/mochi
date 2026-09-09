#pragma once
#include <Mochi.h>

namespace playground {
// Keep leaves and ears inside the same partial-update region as Mochi.
constexpr float stage_scale(mochi::Character character) {
  return character == mochi::Character::Sprout || character == mochi::Character::Peach ? .92f : 1.03f;
}
}
