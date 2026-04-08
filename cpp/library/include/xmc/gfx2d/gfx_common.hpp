#ifndef XMC_GFX_COMMON_HPP
#define XMC_GFX_COMMON_HPP

#include "xmc/xmc_common.hpp"

#include <stdint.h>

namespace xmc {

using RawColor = uint16_t;

static inline uint16_t blend4444(uint16_t a, uint16_t b, int32_t alpha) {
  if (alpha <= 0) {
    return a;
  } else if (alpha >= 256) {
    return b;
  } else {
    uint32_t inv_alpha = 256 - alpha;
    uint32_t ca =
        (((a & 0xF000) * inv_alpha + (b & 0xF000) * alpha) >> 8) & 0xF000;
    uint32_t cr =
        (((a & 0x0F00) * inv_alpha + (b & 0x0F00) * alpha) >> 8) & 0x0F00;
    uint32_t cg =
        (((a & 0x00F0) * inv_alpha + (b & 0x00F0) * alpha) >> 8) & 0x00F0;
    uint32_t cb =
        (((a & 0x000F) * inv_alpha + (b & 0x000F) * alpha) >> 8) & 0x000F;
    return static_cast<uint16_t>(ca | cr | cg | cb);
  }
}

union color4444 {
  uint16_t packed;
  struct {
    uint8_t b : 4;
    uint8_t g : 4;
    uint8_t r : 4;
    uint8_t a : 4;
  };

  color4444() : packed(0) {}
  color4444(uint16_t packed) : packed(packed) {}
  color4444(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
      : b(b), g(g), r(r), a(a) {}
  color4444(int a, int r, int g, int b)
      : b(b < 0 ? 0 : (b > 15 ? 15 : b)),
        g(g < 0 ? 0 : (g > 15 ? 15 : g)),
        r(r < 0 ? 0 : (r > 15 ? 15 : r)),
        a(a < 0 ? 0 : (a > 15 ? 15 : a)) {}
};

static constexpr XMC_INLINE uint16_t pack444(int r, int g, int b) {
  r = xmcClip(0, 15, r);
  g = xmcClip(0, 15, g);
  b = xmcClip(0, 15, b);
  return static_cast<uint16_t>(((r & 0xF) << 8) | ((g & 0xF) << 4) | (b & 0xF));
}

static constexpr XMC_INLINE uint16_t pack565(int r, int g, int b) {
  r = xmcClip(0, 31, r);
  g = xmcClip(0, 63, g);
  b = xmcClip(0, 31, b);
  return static_cast<uint16_t>((r << 3) | ((g & 0x38) >> 3) |
                               ((g & 0x07) << 13) | (b << 8));
}

static XMC_INLINE void unpack565(uint16_t color, int *r, int *g, int *b) {
  color = ((color << 8) & 0xFF00) | ((color >> 8) & 0x00FF);
  *r = (color >> 11) & 0x1F;
  *g = ((color >> 5) & 0x3F);
  *b = color & 0x1F;
}

static XMC_INLINE uint16_t convert444To565(uint16_t color) {
  uint_fast16_t result = 0;
  result |= ((color << 4) & 0xF000) | (color & 0x0800);
  result |= ((color << 3) & 0x0780) | ((color >> 1) & 0x0060);
  result |= ((color << 1) & 0x001E) | ((color >> 3) & 0x0001);
  return ((result << 8) & 0xFFF0) | ((result >> 8) & 0x00FF);
}

static XMC_INLINE uint16_t convert565To444(uint16_t color) {
  uint_fast16_t result = 0;
  color = ((color << 8) & 0xFF00) | ((color >> 8) & 0x00FF);
  result |= (color >> 4) & 0xF00;
  result |= (color >> 3) & 0x0F0;
  result |= (color >> 1) & 0x00F;
  return result;
}

static XMC_INLINE uint8_t convert565ToGray1(uint16_t color) {
  int r, g, b;
  unpack565(color, &r, &g, &b);
  return ((r * 2 + g + b * 2) >= (32 * 3)) ? 1 : 0;
}

static XMC_INLINE uint8_t convert444ToGray1(uint16_t color) {
  uint16_t gray = ((color >> 8) & 0xF) + ((color >> 4) & 0xF) + (color & 0xF);
  return gray >= (8 * 3) ? 1 : 0;
}

static XMC_INLINE uint16_t convert888To565(uint32_t color) {
  uint16_t result = 0;
  result |= (color >> 8) & 0xF800;
  result |= (color >> 5) & 0x7E0;
  result |= (color >> 3) & 0x1F;
  return ((result << 8) & 0xFF00) | ((result >> 8) & 0x00FF);
}

static XMC_INLINE uint16_t convert888To444(uint32_t color) {
  uint16_t result = 0;
  result |= (color >> 12) & 0xF00;
  result |= (color >> 8) & 0xF0;
  result |= (color >> 4) & 0xF;
  return result;
}

static XMC_INLINE uint16_t blend4444To565(uint16_t dest, uint16_t src) {
  uint32_t a2 = src & 0xF000;
  if (a2 == 0xF000) {
    return convert444To565(src);
  } else if (a2 == 0) {
    return dest;
  } else {
    a2 >>= 12;
    uint32_t a1 = 15 - a2;
    uint32_t c2 = ((dest << 8) & 0xFF00) | ((dest >> 8) & 0x00FF);
    c2 = ((c2 & 0xF800) << 9) | ((c2 & 0x07E0) << 5) | (c2 & 0x001F);
    uint32_t c1 =
        ((src & 0xF00) << 13) | ((src & 0x0F0) << 8) | ((src & 0x00F) << 1);
    uint32_t r = c1 * a2 + c2 * a1;
    r = ((r >> 13) & 0xF800) | ((r >> 9) & 0x07E0) | ((r >> 4) & 0x001F);
    return ((r << 8) & 0xFF00) | ((r >> 8) & 0x00FF);
  }
}

static XMC_INLINE uint16_t blend4444To444(uint16_t dest, uint16_t src) {
  uint32_t a2 = src & 0xF000;
  if (a2 == 0xF000) {
    return src & 0x0FFF;
  } else if (a2 == 0) {
    return dest;
  } else {
    a2 >>= 12;
    uint32_t a1 = 15 - a2;
    uint32_t c2 = dest & 0x0FFF;
    c2 = ((c2 & 0xF00) << 8) | ((c2 & 0x0F0) << 4) | (c2 & 0x00F);
    uint32_t c1 = ((src & 0xF00) << 12) | ((src & 0x0F0) << 6) | (src & 0x00F);
    uint32_t r = c1 * a2 + c2 * a1;
    return ((r >> 12) & 0xF00) | ((r >> 8) & 0x0F0) | ((r >> 4) & 0x00F);
  }
}

struct GlyphMetrics {
  int width;
  int height;
  int xOffset;
  int yOffset;
  int xAdvance;
};

enum class TextRenderFlags : uint32_t {
  DRAW_FORE = 1 << 0,
  DRAW_BACK = 1 << 1,
};
XMC_ENUM_FLAGS(TextRenderFlags, uint32_t)

struct TextRenderArgs {
  RawColor foreColor;
  RawColor backColor;
  TextRenderFlags flags;
};

void copyPixelString(PixelFormat dstFmt, void *dst, int dstX,
                     PixelFormat srcFmt, void *src, int srcX, int width,
                     const TextRenderArgs &tra);

}  // namespace xmc

#endif  // XMC_GFX_COMMON_HPP
