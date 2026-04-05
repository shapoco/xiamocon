#include "xmc/system.hpp"
#include "xmc/battery.hpp"
#include "xmc/display.hpp"
#include "xmc/fs.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/i2c.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/power.hpp"
#include "xmc/hw/spi.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/ioex.hpp"
#include "xmc/speaker.hpp"

namespace xmc::system {

uint64_t powerButtonNextReadUs = 0;
uint32_t powerButtonPressedCount = 0;

XmcStatus init() {
  gpio::setDir(XMC_PIN_POWER_BUTTON, false);

  i2c::init();
  spi::init();

  ioex::init();
  ioex::setDirMasked(0, 0xFF, 0xFF);
  ioex::setDirMasked(1, 0xFF, 0xFF);

  battery::init();

  // Mute speaker during initialization to avoid noise
  ioex::write(ioex::Pin::PERI_EN, true);
  ioex::setDir(ioex::Pin::SPEAKER_MUTE, true);

  // Reset LCD
  ioex::write(ioex::Pin::DISPLAY_RESET, false);
  ioex::setDir(ioex::Pin::DISPLAY_RESET, true);

  // Power on peripherals
  ioex::write(ioex::Pin::PERI_EN, true);
  ioex::setDir(ioex::Pin::PERI_EN, true);
  xmc::sleepMs(100);
  ioex::write(ioex::Pin::PERI_EN, false);
  xmc::sleepMs(100);

  return XMC_OK;
}

XmcStatus service() {
  uint64_t now_us = getTimeUs();

  battery::service();
  input::service();

  if (now_us >= powerButtonNextReadUs) {
    powerButtonNextReadUs = now_us + 10000;
    if (gpio::read(XMC_PIN_POWER_BUTTON)) {
      if (powerButtonPressedCount < 3) {
        powerButtonPressedCount++;
      }
    } else {
      if (powerButtonPressedCount >= 3) {
        requestShutdown();
      }
      powerButtonPressedCount = 0;
    }
  }

  return XMC_OK;
}

XmcStatus requestShutdown() {
  display::deinit();
  input::deinit();
  battery::deinit();
  speaker::deinit();
  fs::deinit();

  spi::deinit();

  // shutdown peripherals
  ioex::write(ioex::Pin::PERI_EN, true);
  sleepMs(1000);

  ioex::setDir(ioex::Pin::DISPLAY_RESET, false);
  ioex::setDir(ioex::Pin::SPEAKER_MUTE, false);

  // drain charge from GPIO pins
  gpio::setDir(XMC_PIN_TFCARD_CS, true);
  gpio::setDir(XMC_PIN_DISPLAY_CS, true);
  gpio::setDir(XMC_PIN_DISPLAY_DC, true);
  gpio::setDir(XMC_PIN_AUDIO_OUT, true);
  gpio::setPullup(XMC_PIN_TFCARD_CS, false);
  gpio::setPullup(XMC_PIN_DISPLAY_CS, false);
  gpio::setPullup(XMC_PIN_DISPLAY_DC, false);
  gpio::setPullup(XMC_PIN_AUDIO_OUT, false);
  gpio::write(XMC_PIN_TFCARD_CS, false);
  gpio::write(XMC_PIN_DISPLAY_CS, false);
  gpio::write(XMC_PIN_DISPLAY_DC, false);
  gpio::write(XMC_PIN_AUDIO_OUT, false);
  sleepMs(100);
  gpio::setDir(XMC_PIN_TFCARD_CS, false);
  gpio::setDir(XMC_PIN_DISPLAY_CS, false);
  gpio::setDir(XMC_PIN_DISPLAY_DC, false);
  gpio::setDir(XMC_PIN_AUDIO_OUT, false);

  ioex::deinit();
  i2c::deinit();

  XMC_ERR_RET(power::deepSleep());
  XMC_ERR_RET(power::reset(power::ResetMode::NORMAL));
  return XMC_OK;
}

}  // namespace xmc::system
