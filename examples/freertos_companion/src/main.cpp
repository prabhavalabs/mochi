#include <Arduino.h>
#include <Mochi.h>
#include <MochiWaveshare216.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace board = mochi::waveshare216;
namespace {
constexpr size_t queueDepth = 8;
StaticQueue_t queueControl;
alignas(mochi::Command) uint8_t queueStorage[queueDepth * sizeof(mochi::Command)];
QueueHandle_t commands = nullptr;
mochi::Mascot notification, status;
bool ready = false;
uint32_t previous = 0;
}

// May be called by multiple ordinary application tasks; never blocks.
bool postCompanionCommand(const mochi::Command& command) {
  return commands && xQueueSend(commands, &command, 0) == pdTRUE;
}

// For interrupt handlers, use the RTOS's ISR API and follow its interrupt-priority
// rules. The caller initializes woken=pdFALSE and yields if it becomes pdTRUE.
bool postCompanionCommandFromISR(const mochi::Command& command, BaseType_t* woken) {
  return commands && xQueueSendFromISR(commands, &command, woken) == pdTRUE;
}

void applicationTask(void*) {
  for (;;) {
    // Stand-in for a sensor/network event. The worker never touches UI state.
    vTaskDelay(pdMS_TO_TICKS(5000));
    if (!postCompanionCommand(mochi::Command::trigger(mochi::State::Happy, 1.5f)))
      Serial.println("Companion command queue full");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  commands = xQueueCreateStatic(queueDepth, sizeof(mochi::Command), queueStorage, &queueControl);
  ready = commands && board::begin();
  if (!ready) { Serial.println("Example initialization failed"); return; }
  notification.setCharacter(mochi::Character::Peach);
  status.setCharacter(mochi::Character::Sprout);
  status.setState(mochi::State::Thinking);
  auto& canvas = board::canvas();
  canvas.fillScreen(mochi::kBackground);
  canvas.setTextColor(mochi::rgb565(0xF8F3E6));
  canvas.setTextSize(2);
  canvas.setCursor(24, 24); canvas.print("Your application");
  canvas.setCursor(28, 408); canvas.print("Task events");
  canvas.setCursor(260, 408); canvas.print("Independent");
  board::present();
  if (xTaskCreate(applicationTask, "application", 2048, nullptr, 1, nullptr) != pdPASS)
    Serial.println("Worker creation failed; only the independent widget will animate");
  previous = millis();
}

void loop() {
  delay(1);
  if (!ready) return;
  const uint32_t now = millis();
  if (now - previous < 33) return;
  // Bound work per frame, even if producers continuously post commands.
  mochi::Command command;
  for (size_t n = 0; n < queueDepth && xQueueReceive(commands, &command, 0) == pdTRUE; ++n)
    if (!notification.dispatch(command)) Serial.println("Invalid companion command");
  const float seconds = (now - previous) / 1000.f;
  previous = now;
  notification.update(seconds); status.update(seconds);
  auto& canvas = board::canvas();
  mochi::Surface screen{canvas.getFramebuffer(), 480 * 480, 480, 480};
  auto left = mochi::sub_surface(screen, 16, 80, 216, 300);
  auto right = mochi::sub_surface(screen, 248, 80, 216, 300);
  mochi::Renderer(left).clear(); mochi::Renderer(right).clear();
  notification.draw(left, 108, 150, .53f);
  status.draw(right, 108, 150, .53f);
  board::present(80, 300);
}
