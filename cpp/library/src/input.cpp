#include "xmc/input.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/ioex.hpp"

#include <stddef.h>
#include <stdint.h>

namespace xmc::input {

static uint64_t nextUpdateMs = 0;
static Button currState = Button::NONE;
static Button lastState = Button::NONE;

void init() {
  int num_game_buttons =
      sizeof(ioex::GAME_BUTTON_PINS) / sizeof(ioex::GAME_BUTTON_PINS[0]);
  for (int i = 0; i < num_game_buttons; i++) {
    ioex::setDir(ioex::GAME_BUTTON_PINS[i], false);
  }
  ioex::setDir(ioex::Pin::BTN_FUNC, false);
  currState = Button::NONE;
  lastState = Button::NONE;
}

void deinit() {
  currState = Button::NONE;
  lastState = Button::NONE;
}

XmcStatus service() {
  uint64_t now_ms = xmc::getTimeMs();
  if (now_ms < nextUpdateMs) {
    lastState = currState;
    return XMC_OK;
  }
  nextUpdateMs += 10;
  if (nextUpdateMs < now_ms) {
    nextUpdateMs = now_ms + 10;
  }

  uint16_t tmp;
  if (ioex::tryReadAll(&tmp)) {
    lastState = currState;
    currState = ((Button)~tmp) & Button::ANY;
  }
  return XMC_OK;
}

Button getState() { return currState; }

bool isPressed(Button button) { return !!(currState & button); }

bool wasPressed(Button button) {
  return !!(currState & button) && !(lastState & button);
}

bool wasReleased(Button button) {
  return !(currState & button) && !!(lastState & button);
}

}  // namespace xmc::input
