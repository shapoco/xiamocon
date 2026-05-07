#include "xmc/system.hpp"
#include "xmc/app.hpp"
#include "xmc/battery.hpp"
#include "xmc/display.hpp"
#include "xmc/flash.hpp"
#include "xmc/fs.hpp"
#include "xmc/gfx2d/graphics2d.hpp"
#include "xmc/gpio.hpp"
#include "xmc/i2c.hpp"
#include "xmc/input.hpp"
#include "xmc/ioex.hpp"
#include "xmc/multicore.hpp"
#include "xmc/pins.hpp"
#include "xmc/power.hpp"
#include "xmc/power_off_message.hpp"
#include "xmc/speaker.hpp"
#include "xmc/spi.hpp"
#include "xmc/timer.hpp"

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
  flash::init();

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
        requestShutdown(ShutdownReason::POWER_SWITCH);
      }
      powerButtonPressedCount = 0;
    }
  }

  return XMC_OK;
}

XmcStatus requestShutdown(ShutdownReason reason) {
  XMC_ERR_RET(xmcAppTerminate(reason));

  stopCore1();
  input::deinit();
  battery::deinit();
  flash::deinit();
  speaker::deinit();
  fs::deinit();

  // show power off message
  {
    Graphics2D gfx = createGraphics2D();
    gfx->clear(0);
    Sprite msg = createPowerOffMessageSprite();
    gfx->drawImage(msg, (display::WIDTH - msg->width) / 2,
                   (display::HEIGHT - msg->height) / 2);
    sleepMs(3000);
  }
  display::deinit();
  spi::deinit();

  // shutdown peripherals
  i2c::resetBus();
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
