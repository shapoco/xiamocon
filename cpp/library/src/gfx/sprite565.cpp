#include "xmc/gfx/sprite565.hpp"
#include "xmc/display.hpp"

#include <string.h>

namespace xmc {

void Sprite565Class::onSetPixel(int x, int y, uint16_t color) {
  uint16_t *line = (uint16_t *)linePtr(y);
  line[x] = color;
}

uint16_t Sprite565Class::onGetPixel(int x, int y) const {
  uint16_t *line = (uint16_t *)linePtr(y);
  return line[x];
}

void Sprite565Class::onFillRect(int x, int y, int width, int height,
                                uint16_t color) {
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)linePtr(y + j);
    for (int i = 0; i < width; i++) {
      line[x + i] = color;
    }
  }
}

void Sprite565Class::onFillSmokeRect(int x, int y, int width, int height,
                                     bool light) {
  uint16_t mask = 0xEF7B;
  uint16_t smoke = light ? 0x1084 : 0x0000;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)linePtr(y + j);
    for (int i = 0; i < width; i++) {
      uint16_t c = line[x + i];
      c = (c >> 1) | ((c & 1) << 15);
      c = (c & mask) | smoke;
      line[x + i] = c;
    }
  }
}

void Sprite565Class::onDrawImage(const Sprite &image, int dx, int dy, int w,
                                 int h, int sx, int sy) {
  switch (image->format) {
    default:
      // todo: implement
      break;
  }
}

XmcStatus Sprite565Class::onStartTransferToDisplay(int dx, int dy, int sy,
                                                   int h) {
  XMC_ERR_RET(display::setWindow(dx, dy, width, h));
  XMC_ERR_RET(display::writePixelsStart(linePtr(sy), stride * h, false));
  return XMC_OK;
}

}  // namespace xmc