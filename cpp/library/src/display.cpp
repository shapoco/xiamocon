#include "xmc/display.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/spi.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/ioex.hpp"

namespace xmc::display {

typedef enum {
  XMC_ST7789_NOP = 0x00,
  XMC_ST7789_SOFTWARE_RESET = 0x01,
  XMC_ST7789_READ_DISP_ID = 0x04,
  XMC_ST7789_READ_ERROR_DSI = 0x05,
  XMC_ST7789_READ_DISP_STATUS = 0x09,
  XMC_ST7789_READ_DISP_POWER_MODE = 0x0A,
  XMC_ST7789_READ_DISP_MADCTRL = 0x0B,
  XMC_ST7789_READ_DISP_PIXEL_FORMAT = 0x0C,
  XMC_ST7789_READ_DISP_IMAGE_MODE = 0x0D,
  XMC_ST7789_READ_DISP_SIGNAL_MODE = 0x0E,
  XMC_ST7789_READ_DISP_SELF_DIAGNOSTIC = 0x0F,
  XMC_ST7789_ENTER_SLEEP_MODE = 0x10,
  XMC_ST7789_SLEEP_OUT = 0x11,
  XMC_ST7789_PARTIAL_MODE_ON = 0x12,
  XMC_ST7789_NORMAL_DISP_MODE_ON = 0x13,
  XMC_ST7789_DISP_INVERSION_OFF = 0x20,
  XMC_ST7789_DISP_INVERSION_ON = 0x21,
  XMC_ST7789_PIXEL_OFF = 0x22,
  XMC_ST7789_PIXEL_ON = 0x23,
  XMC_ST7789_DISPLAY_OFF = 0x28,
  XMC_ST7789_DISPLAY_ON = 0x29,
  XMC_ST7789_COLUMN_ADDRESS_SET = 0x2A,
  XMC_ST7789_PAGE_ADDRESS_SET = 0x2B,
  XMC_ST7789_MEMORY_WRITE = 0x2C,
  XMC_ST7789_MEMORY_READ = 0x2E,
  XMC_ST7789_PARTIAL_AREA = 0x30,
  XMC_ST7789_VERT_SCROLL_DEFINITION = 0x33,
  XMC_ST7789_TEARING_EFFECT_LINE_OFF = 0x34,
  XMC_ST7789_TEARING_EFFECT_LINE_ON = 0x35,
  XMC_ST7789_MEMORY_ACCESS_CONTROL = 0x36,
  XMC_ST7789_VERT_SCROLL_START_ADDRESS = 0x37,
  XMC_ST7789_IDLE_MODE_OFF = 0x38,
  XMC_ST7789_IDLE_MODE_ON = 0x39,
  XMC_ST7789_INTERFACE_PIXEL_FORMAT = 0x3A,
  XMC_ST7789_MEMORY_WRITE_CONTINUE = 0x3C,
  XMC_ST7789_MEMORY_READ_CONTINUE = 0x3E,
  XMC_ST7789_SET_TEAR_SCANLINE = 0x44,
  XMC_ST7789_GET_SCANLINE = 0x45,
  XMC_ST7789_WRITE_DISPLAY_BRIGHTNESS = 0x51,
  XMC_ST7789_READ_DISPLAY_BRIGHTNESS = 0x52,
  XMC_ST7789_WRITE_CTRL_DISPLAY = 0x53,
  XMC_ST7789_READ_CTRL_DISPLAY = 0x54,
  XMC_ST7789_WRITE_CONTENT_ADAPT_BRIGHTNESS = 0x55,
  XMC_ST7789_READ_CONTENT_ADAPT_BRIGHTNESS = 0x56,
  XMC_ST7789_WRITE_MIN_CAB_LEVEL = 0x5E,
  XMC_ST7789_READ_MIN_CAB_LEVEL = 0x5F,
  XMC_ST7789_READ_ABC_SELF_DIAG_RES = 0x68,
  XMC_ST7789_READ_ID1 = 0xDA,
  XMC_ST7789_READ_ID2 = 0xDB,
  XMC_ST7789_READ_ID3 = 0xDC,
  XMC_ST7789_INTERFACE_MODE_CONTROL = 0xB0,
  XMC_ST7789_FRAME_RATE_CONTROL_NORMAL = 0xB1,
  XMC_ST7789_FRAME_RATE_CONTROL_IDLE_8COLOR = 0xB2,
  XMC_ST7789_FRAME_RATE_CONTROL_PARTIAL = 0xB3,
  XMC_ST7789_DISPLAY_INVERSION_CONTROL = 0xB4,
  XMC_ST7789_BLANKING_PORCH_CONTROL = 0xB5,
  XMC_ST7789_DISPLAY_FUNCTION_CONTROL = 0xB6,
  XMC_ST7789_ENTRY_MODE_SET = 0xB7,
  XMC_ST7789_BACKLIGHT_CONTROL_1 = 0xB9,
  XMC_ST7789_BACKLIGHT_CONTROL_2 = 0xBA,
  XMC_ST7789_HS_LANES_CONTROL = 0xBE,
  XMC_ST7789_POWER_CONTROL_1 = 0xC0,
  XMC_ST7789_POWER_CONTROL_2 = 0xC1,
  XMC_ST7789_POWER_CONTROL_NORMAL_3 = 0xC2,
  XMC_ST7789_POWER_CONTROL_IDEL_4 = 0xC3,
  XMC_ST7789_POWER_CONTROL_PARTIAL_5 = 0xC4,
  XMC_ST7789_VCOM_CONTROL_1 = 0xC5,
  XMC_ST7789_CABC_CONTROL_1 = 0xC6,
  XMC_ST7789_CABC_CONTROL_2 = 0xC8,
  XMC_ST7789_CABC_CONTROL_3 = 0xC9,
  XMC_ST7789_CABC_CONTROL_4 = 0xCA,
  XMC_ST7789_CABC_CONTROL_5 = 0xCB,
  XMC_ST7789_CABC_CONTROL_6 = 0xCC,
  XMC_ST7789_CABC_CONTROL_7 = 0xCD,
  XMC_ST7789_CABC_CONTROL_8 = 0xCE,
  XMC_ST7789_CABC_CONTROL_9 = 0xCF,
  XMC_ST7789_NVMEM_WRITE = 0xD0,
  XMC_ST7789_NVMEM_PROTECTION_KEY = 0xD1,
  XMC_ST7789_NVMEM_STATUS_READ = 0xD2,
  XMC_ST7789_READ_ID4 = 0xD3,
  XMC_ST7789_ADJUST_CONTROL_1 = 0xD7,
  XMC_ST7789_READ_ID_VERSION = 0xD8,
  XMC_ST7789_POSITIVE_GAMMA_CORRECTION = 0xE0,
  XMC_ST7789_NEGATIVE_GAMMA_CORRECTION = 0xE1,
  XMC_ST7789_DIGITAL_GAMMA_CONTROL_1 = 0xE2,
  XMC_ST7789_DIGITAL_GAMMA_CONTROL_2 = 0xE3,
  XMC_ST7789_SET_IMAGE_FUNCTION = 0xE9,
  XMC_ST7789_ADJUST_CONTROL_2 = 0xF2,
  XMC_ST7789_ADJUST_CONTROL_3 = 0xF7,
  XMC_ST7789_ADJUST_CONTROL_4 = 0xF8,
  XMC_ST7789_ADJUST_CONTROL_5 = 0xF9,
  XMC_ST7789_SPI_READ_COMMAND_SETTING = 0xFB,
  XMC_ST7789_ADJUST_CONTROL_6 = 0xFC,
  XMC_ST7789_ADJUST_CONTROL_7 = 0xFF,
} xmc_st7789_command_t;

static PixelFormat currentFormat = PixelFormat::RGB444;
static bool inTransaction = false;

// max 3 bytes per pixel for RGB666
static uint8_t lineBuffer[WIDTH * 3];

static XmcStatus begin_command(uint8_t cmd);
static void end_command();
static XmcStatus write_data(const uint8_t *data, uint32_t size);

XmcStatus init(PixelFormat format, int rotation) {
  currentFormat = format;

  gpio::setDir(XMC_PIN_DISPLAY_CS, true);
  gpio::setDir(XMC_PIN_DISPLAY_DC, true);
  gpio::write(XMC_PIN_DISPLAY_CS, 1);
  gpio::write(XMC_PIN_DISPLAY_DC, 1);

  XMC_ERR_RET(ioex::setDir(ioex::Pin::DISPLAY_RESET, true));
  XMC_ERR_RET(ioex::write(ioex::Pin::DISPLAY_RESET, 0));
  sleepMs(100);
  XMC_ERR_RET(ioex::write(ioex::Pin::DISPLAY_RESET, 1));
  sleepMs(100);

  XMC_ERR_RET(writeCommandNoParam(XMC_ST7789_SOFTWARE_RESET));
  sleepMs(200);

  XMC_ERR_RET(writeCommandNoParam(XMC_ST7789_SLEEP_OUT));
  sleepMs(200);

  uint8_t intf_format;
  switch (currentFormat) {
    case PixelFormat::RGB444: intf_format = 0x53; break;
    case PixelFormat::RGB565: intf_format = 0x55; break;
    default: return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
  }
  XMC_ERR_RET(
      writeCommand1Param(XMC_ST7789_INTERFACE_PIXEL_FORMAT, intf_format));

  // todo: support rotation
  // #if 0
  //     writeCommand(Command::MEMORY_ACCESS_CONTROL, 0x48);
  // #else
  //     // writeCommand(Command::MEMORY_ACCESS_CONTROL, 0xE8);
  //     writeCommand(Command::MEMORY_ACCESS_CONTROL, 0x28);
  // #endif

  if (0) {
    uint8_t params0[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F,
                         0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
    uint8_t params1[] = {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x32,
                         0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17};
    uint8_t params[14];
    for (int i = 0; i < 14; i++) {
      params[i] = (params0[i] + params1[i]) / 2;
    }
    XMC_ERR_RET(writeCommand(XMC_ST7789_POSITIVE_GAMMA_CORRECTION, params,
                             sizeof(params)));
  }

  if (0) {
    uint8_t params0[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F,
                         0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
    uint8_t params1[] = {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x31,
                         0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E};
    uint8_t params[14];
    for (int i = 0; i < 14; i++) {
      params[i] = (params0[i] + params1[i]) / 2;
    }
    XMC_ERR_RET(writeCommand(XMC_ST7789_NEGATIVE_GAMMA_CORRECTION, params,
                             sizeof(params)));
  }

  XMC_ERR_RET(writeCommandNoParam(XMC_ST7789_DISP_INVERSION_ON));

  XMC_ERR_RET(fillRect(0, 0, WIDTH, HEIGHT, 0x0000));

  XMC_ERR_RET(writeCommandNoParam(XMC_ST7789_DISPLAY_ON));
  sleepMs(25);
  return XMC_OK;
}

XmcStatus deinit() {
  XMC_ERR_RET(ioex::write(ioex::Pin::DISPLAY_RESET, 0));
  return XMC_OK;
}

PixelFormat getPixelFormat() { return currentFormat; }

XmcStatus clear(uint32_t color) {
  XMC_ERR_RET(fillRect(0, 0, WIDTH, HEIGHT, color));
  return XMC_OK;
}

XmcStatus fillRect(int x, int y, int width, int height, uint32_t color) {
  if (x < 0) {
    width += x;
    x = 0;
    if (width <= 0) return XMC_OK;
  }
  if (x + width > WIDTH) {
    width = WIDTH - x;
    if (width <= 0) return XMC_OK;
  }
  if (y < 0) {
    height += y;
    y = 0;
    if (height <= 0) return XMC_OK;
  }
  if (y + height > HEIGHT) {
    height = HEIGHT - y;
    if (height <= 0) return XMC_OK;
  }

  int lineBytes;
  switch (currentFormat) {
    case PixelFormat::RGB444: {
      lineBytes = width * 3 / 2;
      for (int i = 0; i < lineBytes; i += 3) {
        lineBuffer[i + 0] = (color >> 4) & 0xFF;
        lineBuffer[i + 1] = ((color & 0x0F) << 4) | ((color >> 8) & 0x0F);
        lineBuffer[i + 2] = color & 0xFF;
      }
    } break;
    case PixelFormat::RGB565: {
      lineBytes = width * 2;
      uint16_t *lineBuffer565 = (uint16_t *)lineBuffer;
      for (int i = 0; i < width; i++) {
        lineBuffer565[i] = color;
      }
    } break;
    default: return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
  }

  for (int i = 0; i < height; i++) {
    XMC_ERR_RET(setWindow(x, y + i, width, 1));
    XMC_ERR_RET(writePixelsStart(lineBuffer, lineBytes, false));
    XMC_ERR_RET(writePixelsComplete());
  }
  return XMC_OK;
}

XmcStatus setWindow(int x, int y, int width, int height) {
  int x_end = x + width - 1;
  int y_end = y + height - 1;
  XMC_ERR_RET(writeCommand4Params(XMC_ST7789_COLUMN_ADDRESS_SET, x >> 8,
                                  x & 0xFF, x_end >> 8, x_end & 0xFF));
  XMC_ERR_RET(writeCommand4Params(XMC_ST7789_PAGE_ADDRESS_SET, y >> 8, y & 0xFF,
                                  y_end >> 8, y_end & 0xFF));
  return XMC_OK;
}

XmcStatus writePixelsStart(const void *data, uint32_t num_bytes,
                           bool repeated) {
  XMC_ERR_RET(begin_command(XMC_ST7789_MEMORY_WRITE));
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  dma::Config cfg = {
      .ptr = (void *)data,
      .elementSize = 1,
      .length = (int)num_bytes,
  };
  XmcStatus sts = spi::dmaWriteStart(&cfg, XMC_PIN_DISPLAY_CS);
  if (sts != XMC_OK) {
    end_command();
  }
  return sts;
}

XmcStatus writePixelsComplete() {
  XmcStatus sts = spi::dmaComplete();
  end_command();
  return sts;
}

XmcStatus writeCommand(const uint8_t cmd, const uint8_t *params,
                       uint32_t numParams) {
  XMC_ERR_RET(begin_command(cmd));
  XmcStatus sts = XMC_OK;
  if (numParams > 0) {
    sts = write_data(params, numParams);
  }
  end_command();
  return sts;
}

static XmcStatus begin_command(uint8_t cmd) {
  XMC_ERR_RET(spi::lock());
  inTransaction = true;
  spi::setBaudrate(spi::getPreferredFrequency(Chipset::DISPLAY));
  XmcStatus sts = XMC_OK;
  do {
    gpio::write(XMC_PIN_DISPLAY_DC, 0);
    gpio::write(XMC_PIN_DISPLAY_CS, 0);
    XMC_ERR_BRK(sts, spi::writeBlocking(&cmd, 1));
  } while (0);
  if (sts != XMC_OK) {
    end_command();
  }
  return sts;
}

static void end_command() {
  if (!inTransaction) return;
  inTransaction = false;
  gpio::write(XMC_PIN_DISPLAY_CS, 1);
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  spi::unlock();
}

static XmcStatus write_data(const uint8_t *data, uint32_t size) {
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  return spi::writeBlocking(data, size);
}

}  // namespace xmc::display
