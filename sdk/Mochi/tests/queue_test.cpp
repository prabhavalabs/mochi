#include <mochi/CommandQueue.h>
#include <cassert>
#include <thread>

int main() {
  using namespace mochi;
  CommandQueue<2> small;
  Command command;
  assert(!small.pop(command));
  assert(small.push(Command::setState(State::Thinking)));
  assert(small.push(Command::setCharacter(Character::Peach)));
  assert(!small.push(Command::blink()));
  assert(small.pop(command) && command.type == CommandType::SetState && command.state == State::Thinking);
  assert(small.push(Command::trigger(State::Happy, 1)));
  assert(small.pop(command) && command.character == Character::Peach);
  assert(small.pop(command) && command.type == CommandType::Trigger && command.value == 1);
  assert(!small.pop(command));
  CommandQueue<1> one;
  assert(one.push(Command::blink()) && !one.push(Command::blink()));
  assert(one.pop(command) && one.push(Command::blink()));

  CommandQueue<7> concurrent;
  constexpr int total = 100000;
  std::thread producer([&] {
    for (int i = 0; i < total; ++i) {
      Command next = Command::trigger(State::Happy, float(i));
      while (!concurrent.push(next)) std::this_thread::yield();
    }
  });
  for (int i = 0; i < total; ++i) {
    while (!concurrent.pop(command)) std::this_thread::yield();
    assert(command.type == CommandType::Trigger && command.state == State::Happy && command.value == float(i));
  }
  producer.join();
  assert(!concurrent.pop(command));
}
