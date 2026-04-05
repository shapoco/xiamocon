#include "xmc/gfx2d/gfx_common.hpp"
#include "xmc/gfx2d/raster_scan.hpp"

#include <string.h>

namespace xmc {

void copyPixelString(PixelFormat dstFmt, void *dst, int dstX,
                     PixelFormat srcFmt, void *src, int srcX, int width,
                     const TextRenderArgs &tra) {
  switch (srcFmt) {
    case PixelFormat::GRAY1: {
      RasterScanGray1 srcScan((uint8_t *)src, srcX);
      switch (dstFmt) {
        case PixelFormat::GRAY1: {
          RasterScanGray1 dstScan((uint8_t *)dst, dstX);
          // todo: use memcpy if possible
          for (int i = 0; i < width; i++) {
            dstScan.pushGray1(srcScan.pop(), tra);
          }
        } break;
        case PixelFormat::RGB444: {
          RasterScan444 dstScan((uint8_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.pushGray1(srcScan.pop(), tra);
          }
        } break;
        case PixelFormat::RGB565: {
          RasterScan565 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.pushGray1(srcScan.pop(), tra);
          }
        } break;
        case PixelFormat::ARGB4444: {
          RasterScan4444 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.pushGray1(srcScan.pop(), tra);
          }
        } break;
        default: break;
      }
    } break;

    case PixelFormat::RGB444: {
      RasterScan444 srcScan((uint8_t *)src, srcX);
      switch (dstFmt) {
        case PixelFormat::GRAY1: {
          RasterScanGray1 dstScan((uint8_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push444(srcScan.pop());
          }
        } break;
        case PixelFormat::RGB444: {
          RasterScan444 dstScan((uint8_t *)dst, dstX);
          // todo: use memcpy if possible
          for (int i = 0; i < width; i++) {
            dstScan.push444(srcScan.pop());
          }
        } break;
        case PixelFormat::RGB565: {
          RasterScan565 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push444(srcScan.pop());
          }
        } break;
        case PixelFormat::ARGB4444: {
          RasterScan4444 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push444(srcScan.pop());
          }
        } break;
        default: break;
      }
    } break;

    case PixelFormat::RGB565: {
      RasterScan565 srcScan((uint16_t *)src, srcX);
      switch (dstFmt) {
        case PixelFormat::GRAY1: {
          RasterScanGray1 dstScan((uint8_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push565(srcScan.pop());
          }
        } break;
        case PixelFormat::RGB444: {
          RasterScan444 dstScan((uint8_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push565(srcScan.pop());
          }
        } break;
        case PixelFormat::RGB565: {
          uint16_t *src565 = (uint16_t *)src + srcX;
          uint16_t *dst565 = (uint16_t *)dst + dstX;
          memcpy(dst565, src565, width * sizeof(uint16_t));
        } break;
        case PixelFormat::ARGB4444: {
          RasterScan4444 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push565(srcScan.pop());
          }
        } break;
        default: break;
      }
    } break;

    case PixelFormat::ARGB4444: {
      RasterScan4444 srcScan((uint16_t *)src, srcX);
      switch (dstFmt) {
        case PixelFormat::GRAY1: {
          RasterScanGray1 dstScan((uint8_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push4444(srcScan.pop());
          }
        } break;
        case PixelFormat::RGB444: {
          RasterScan444 dstScan((uint8_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push4444(srcScan.pop());
          }
        } break;
        case PixelFormat::RGB565: {
          RasterScan565 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push4444(srcScan.pop());
          }
        } break;
        case PixelFormat::ARGB4444: {
          RasterScan4444 dstScan((uint16_t *)dst, dstX);
          for (int i = 0; i < width; i++) {
            dstScan.push4444(srcScan.pop());
          }
        } break;
        default: break;
      }
    } break;
    default: break;
  }
}

}  // namespace xmc
