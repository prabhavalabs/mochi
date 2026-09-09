#include "mochi/Character.h"

namespace mochi {
const char* character_name(Character character) {
  switch (character) {
    case Character::Mochi: return "Mochi";
    case Character::Sprout: return "Sprout";
    case Character::Peach: return "Peach";
    case Character::Nimbus: return "Nimbus";
    default: return "Unknown";
  }
}
const char* character_detail(Character character) {
  switch (character) {
    case Character::Mochi: return "Your soft little buddy";
    case Character::Sprout: return "A curious little seedling";
    case Character::Peach: return "A sweet little cat";
    case Character::Nimbus: return "A dreamy little cloud";
    default: return "";
  }
}
}
