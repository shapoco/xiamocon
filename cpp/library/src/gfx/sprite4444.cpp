#include "xmc/gfx2d/sprite4444.hpp"
#include "xmc/display.hpp"

#include <string.h>

namespace xmc {

void Sprite4444Class::onSetPixel(int x, int y, uint16_t color) {
  uint16_t *line = (uint16_t *)linePtr(y);
  line[x] = color;
}

uint16_t Sprite4444Class::onGetPixel(int x, int y) const {
  uint16_t *line = (uint16_t *)linePtr(y);
  return line[x];
}

void Sprite4444Class::onFillRect(int x, int y, int width, int height,
                                 uint16_t color) {
  if (width <= 0 || height <= 0) return;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)linePtr(y + j);
    for (int i = 0; i < width; i++) {
      line[x + i] = color;
    }
  }
}

void Sprite4444Class::onFillSmokeRect(int x, int y, int width, int height,
                                     bool light) {
  uint16_t mask = 0x0777;
  uint16_t smoke = light ? 0x0888 : 0x0000;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)linePtr(y + j);
    for (int i = 0; i < width; i++) {
      uint16_t c = line[x + i];
      uint16_t a = c & 0xF000;
      c = (c & mask) | smoke;
      line[x + i] = c | a;
    }
  }
}

void Sprite4444Class::onDrawImage(const Sprite &image, int dx, int dy, int w,
                                  int h, int sx, int sy) {
  switch (image->format) {
    default:
      // todo: implement
      break;
  }
}

XmcStatus Sprite4444Class::onStartTransferToDisplay(int dx, int dy, int sy,
                                                        int h) {
  return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
}

}  // namespace xmc