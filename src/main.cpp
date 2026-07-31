// InkRecipes firmware entry point for the Xteink X4/X3 (ESP32-C3).
// TODO(hardware-test): whole boot path; see docs/HARDWARE_TESTING.md.
#ifdef ARDUINO

#include <Arduino.h>
#include <HalClock.h>
#include <HalDisplay.h>
#include <HalGPIO.h>
#include <HalPowerManager.h>
#include <HalStorage.h>
#include <Logging.h>

#include <inkkit/inkkit.h>

#include "device/InkRecipesApp.h"
#include "device/TextRenderer.h"

namespace {

inkkit::Display g_display(display);
inkkit::Buttons g_buttons(gpio);
inkrecipes::TextRenderer g_tr(g_display);
inkrecipes::InkRecipesApp g_app(g_tr, g_buttons);

}  // namespace

void setup() {
  Serial.begin(115200);
  LOG_INF("IR", "InkRecipes starting");

  gpio.begin();
  Storage.begin();
  g_display.begin();
  halClock.begin();
  powerManager.begin();

  g_app.begin();
}

void loop() {
  g_app.tick();

  // Cooking mode keeps the device awake; only an explicit power hold sleeps,
  // and even that is ignored mid-cook so a pocket press cannot kill a timer.
  if (g_buttons.powerHeldMs() >= 2000 && !g_app.keepAwake()) {
    inkkit::Power(powerManager, display, gpio).deepSleep();
  }
  delay(20);
}

#endif  // ARDUINO
