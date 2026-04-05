#include "xmc/gfx2d/sprite444.hpp"
#include "xmc/display.hpp"
#include "xmc/geo.hpp"
#include "xmc/gfx2d/sprite4444.hpp"

#include <string.h>
#include <memory>

namespace xmc {

void Sprite444Class::onSetPixel(int x, int y, uint16_t color) {
  Scanner444 wrPtr(*this, x, y);
  wrPtr.push444(color);
}

uint16_t Sprite444Class::onGetPixel(int x, int y) const {
  uint8_t *line = (uint8_t *)linePtr(y);
  int i = x / 2 * 3;
  if ((x & 1) == 0) {
    return ((uint16_t)line[i + 0] << 4) | (line[i + 1] >> 4);
  } else {
    return ((uint16_t)(line[i + 1] & 0x0F) << 8) | line[i + 2];
  }
}

void Sprite444Class::onFillRect(int x, int y, int width, int height,
                                uint16_t color) {
  if (width <= 0 || height <= 0) return;

  Scanner444 wrPtr(*this, x, y);
  for (int i = 0; i < width; i++) {
    wrPtr.push444(color);
  }
  int copyL = (x + 1) & 0xFFFFFFFE;
  int copyR = (x + width) & 0xFFFFFFFE;
  int copyOffset = copyL * 3 / 2;
  int copyBytes = (copyR - copyL) * 3 / 2;
  uint8_t *copySrc = (uint8_t *)linePtr(y) + copyOffset;
  for (int j = 1; j < height; j++) {
    uint8_t *copyDst = (uint8_t *)linePtr(y + j) + copyOffset;
    if (copyBytes > 0) {
      memcpy(copyDst, copySrc, copyBytes);
    }
    if (x & 1) {
      onSetPixel(x, y + j, color);
    }
    if (((x + width) & 1)) {
      onSetPixel(x + width - 1, y + j, color);
    }
  }
}

void Sprite444Class::onFillSmokeRect(int x, int y, int width, int height,
                                     bool light) {
  uint16_t mask = 0x0777;
  uint16_t smoke = light ? 0x0888 : 0x0000;
  for (int j = 0; j < height; j++) {
    Scanner444 ptr(*this, x, y + j);
    for (int i = 0; i < width; i++) {
      uint16_t col = ptr.peek();
      col = ((col >> 1) & mask) | smoke;
      ptr.push444(col);
    }
  }
}

void Sprite444Class::onDrawImage(const Sprite &image, int dx, int dy, int w,
                                 int h, int sx, int sy) {
  switch (image->format) {
    case PixelFormat::ARGB4444:
      for (int j = 0; j < h; j++) {
        const uint16_t *rdPtr = (const uint16_t *)image->linePtr(sy + j) + sx;
        Scanner444 wrPtr(*this, dx, dy + j);
        for (int i = 0; i < w; i++) {
          wrPtr.push4444(*(rdPtr++));
        }
      }
      break;

    default:
      // todo: implement
      break;
  }
}

XmcStatus Sprite444Class::onStartTransferToDisplay(int dx, int dy, int sy,
                                                   int h) {
  XMC_ERR_RET(display::setWindow(dx, dy, width, h));
  XMC_ERR_RET(display::writePixelsStart(linePtr(sy), stride * h, false));
  return XMC_OK;
}

}  // namespace xmc
