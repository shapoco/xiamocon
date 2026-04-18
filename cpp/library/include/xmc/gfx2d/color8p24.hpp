#ifndef XMC_GFX_COLOR8P24_HPP
#define XMC_GFX_COLOR8P24_HPP

#include "xmc/ap_fixed.hpp"
#include "xmc/gfx2d/colorf.hpp"
#include "xmc/gfx2d/gfx_common.hpp"

namespace xmc {

struct color8p24 {
  static constexpr int PRECISION = 24;
  static constexpr int ONE = 1 << PRECISION;
  fixed8p24 a;
  fixed8p24 r;
  fixed8p24 g;
  fixed8p24 b;

  XMC_INLINE color8p24() : a(0), r(0), g(0), b(0) {}
  XMC_INLINE color8p24(fixed8p24 a, fixed8p24 r, fixed8p24 g, fixed8p24 b)
      : a(a), r(r), g(g), b(b) {}
  XMC_INLINE color8p24(colorf c)
      : a(fixed8p24::fromFloat(c.a)),
        r(fixed8p24::fromFloat(c.r)),
        g(fixed8p24::fromFloat(c.g)),
        b(fixed8p24::fromFloat(c.b)) {}

  static XMC_INLINE uint32_t shiftCh(fixed8p24 val, int outWidth) {
    if (outWidth > PRECISION) {
      return val.raw << (outWidth - PRECISION);
    } else if (outWidth < PRECISION) {
      return val.raw >> (PRECISION - outWidth);
    } else {
      return val.raw;
    }
  }

  XMC_INLINE static color8p24 from4444(uint16_t c) {
    uint16_t a = (c >> 12) & 0xF;
    uint16_t r = (c >> 8) & 0xF;
    uint16_t g = (c >> 4) & 0xF;
    uint16_t b = c & 0xF;
    color8p24 result;
    result.a.raw = a * (ONE / 15);
    result.r.raw = r * (ONE / 15);
    result.g.raw = g * (ONE / 15);
    result.b.raw = b * (ONE / 15);
    return result;
  }

  XMC_INLINE uint16_t to565() const {
    uint_fast16_t result = 0;
    if (r.raw >= ONE) {
      result = 0xF800;
    } else if (r.raw >= 0) {
      result = shiftCh(r, 16) & 0xF800;
    }
    if (g.raw >= ONE) {
      result |= 0x07E0;
    } else if (g.raw >= 0) {
      result |= shiftCh(g, 11) & 0x07E0;
    }
    if (b.raw >= ONE) {
      result |= 0x001F;
    } else if (b.raw >= 0) {
      result |= shiftCh(b, 5) & 0x001F;
    }
    return ((result << 8) & 0xFF00) | ((result >> 8) & 0x00FF);
  }

  XMC_INLINE uint16_t to444() const {
    uint_fast16_t result = 0;
    if (r.raw >= ONE) {
      result = 0x0F00;
    } else if (r.raw >= 0) {
      result = shiftCh(r, 12) & 0x0F00;
    }
    if (g.raw >= ONE) {
      result |= 0x00F0;
    } else if (g.raw >= 0) {
      result |= shiftCh(g, 8) & 0x00F0;
    }
    if (b.raw >= ONE) {
      result |= 0x000F;
    } else if (b.raw >= 0) {
      result |= shiftCh(b, 4) & 0x000F;
    }
    return result;
  }

  XMC_INLINE uint16_t to4444() const {
    uint_fast16_t result = 0;
    if (a.raw >= ONE) {
      result |= 0xF000;
    } else if (a.raw >= 0) {
      result |= shiftCh(a, 16) & 0xF000;
    }
    if (r.raw >= ONE) {
      result |= 0x0F00;
    } else if (r.raw >= 0) {
      result |= shiftCh(r, 12) & 0x0F00;
    }
    if (g.raw >= ONE) {
      result |= 0x00F0;
    } else if (g.raw >= 0) {
      result |= shiftCh(g, 8) & 0x00F0;
    }
    if (b.raw >= ONE) {
      result |= 0x000F;
    } else if (b.raw >= 0) {
      result |= shiftCh(b, 4) & 0x000F;
    }
    return result;
  }

  XMC_INLINE color8p24 operator+(const color8p24 &other) const {
    return color8p24{a + other.a, r + other.r, g + other.g, b + other.b};
  }

  XMC_INLINE color8p24 operator-(const color8p24 &other) const {
    return color8p24{a - other.a, r - other.r, g - other.g, b - other.b};
  }

  XMC_INLINE color8p24 operator*(float scalar) const {
    return color8p24{a * scalar, r * scalar, g * scalar, b * scalar};
  }

  XMC_INLINE color8p24 operator*(const color8p24 &other) const {
    return color8p24{a * other.a, r * other.r, g * other.g, b * other.b};
  }

  XMC_INLINE color8p24 operator/(float scalar) const {
    return color8p24{a / scalar, r / scalar, g / scalar, b / scalar};
  }

  XMC_INLINE color8p24 operator/(int32_t scalar) const {
    return color8p24{a / scalar, r / scalar, g / scalar, b / scalar};
  }

  XMC_INLINE color8p24 &operator+=(const color8p24 &other) {
    a += other.a;
    r += other.r;
    g += other.g;
    b += other.b;
    return *this;
  }

  XMC_INLINE color8p24 &operator-=(const color8p24 &other) {
    a -= other.a;
    r -= other.r;
    g -= other.g;
    b -= other.b;
    return *this;
  }

  XMC_INLINE color8p24 &operator*=(float scalar) {
    a *= scalar;
    r *= scalar;
    g *= scalar;
    b *= scalar;
    return *this;
  }

  XMC_INLINE color8p24 &operator*=(int32_t scalar) {
    a *= scalar;
    r *= scalar;
    g *= scalar;
    b *= scalar;
    return *this;
  }

  XMC_INLINE color8p24 &operator*=(const color8p24 &other) {
    a *= other.a;
    r *= other.r;
    g *= other.g;
    b *= other.b;
    return *this;
  }

  XMC_INLINE color8p24 &operator/=(float scalar) {
    a /= scalar;
    r /= scalar;
    g /= scalar;
    b /= scalar;
    return *this;
  }

  XMC_INLINE color8p24 &operator/=(int32_t scalar) {
    a /= scalar;
    r /= scalar;
    g /= scalar;
    b /= scalar;
    return *this;
  }

  XMC_INLINE void multSelf565(uint16_t other) {
    int32_t rOther = (other >> 3) & 0x1F;
    int32_t gOther = ((other << 3) & 0x38) | ((other >> 13) & 0x7);
    int32_t bOther = (other >> 8) & 0x1F;
    r.raw = ((r.raw >> 8) * rOther * 132) >> 4;
    g.raw = ((g.raw >> 8) * gOther * 65) >> 4;
    b.raw = ((b.raw >> 8) * bOther * 132) >> 4;
  }

  XMC_INLINE void multSelf4444(uint16_t other) {
    a.raw = ((a.raw >> 8) * ((other >> 12) & 0xF) * 273) >> 4;
    r.raw = ((r.raw >> 8) * ((other >> 8) & 0xF) * 273) >> 4;
    g.raw = ((g.raw >> 8) * ((other >> 4) & 0xF) * 273) >> 4;
    b.raw = ((b.raw >> 8) * (other & 0xF) * 273) >> 4;
  }
};

static XMC_INLINE uint16_t blend8p24To565(uint16_t dest, color8p24 src) {
  int32_t a1 = src.a.raw;
  if (a1 <= 0) {
    return dest;
  } else if (a1 >= color8p24::ONE) {
    return src.to565();
  } else {
    a1 = color8p24::shiftCh(a1, 5);
    int32_t a2 = 32 - a1;
    uint32_t c2 = ((dest << 8) & 0xFF00) | ((dest >> 8) & 0x00FF);
    c2 = ((c2 << 10) & 0x7C00000) | ((c2 << 5) & 0xFC00) | (c2 & 0x1F);
    uint32_t c1r = XMC_CLIP(0, color8p24::ONE - 1, src.r.raw);
    uint32_t c1g = XMC_CLIP(0, color8p24::ONE - 1, src.g.raw);
    uint32_t c1b = XMC_CLIP(0, color8p24::ONE - 1, src.b.raw);
    uint32_t c1 = (color8p24::shiftCh(c1r, 26) & 0x7C00000) |
                  (color8p24::shiftCh(c1g, 16) & 0xFC00) |
                  (color8p24::shiftCh(c1b, 5) & 0x1F);
    uint32_t r = c1 * a1 + c2 * a2;
    r = ((r >> 16) & 0xF800) | ((r >> 10) & 0x07E0) | ((r >> 5) & 0x001F);
    return ((r << 8) & 0xFF00) | ((r >> 8) & 0x00FF);
  }
}

static XMC_INLINE uint16_t blend8p24To444(uint16_t dest, color8p24 src) {
  int32_t a1 = src.a.raw;
  if (a1 <= 0) {
    return dest;
  } else if (a1 >= color8p24::ONE) {
    return src.to444();
  } else {
    a1 = color8p24::shiftCh(a1, 4);
    int32_t a2 = 16 - a1;
    uint32_t c2 =
        ((dest & 0xF00) << 8) | ((dest & 0x0F0) << 4) | (dest & 0x00F);
    uint32_t c1r = XMC_CLIP(0, color8p24::ONE - 1, src.r.raw);
    uint32_t c1g = XMC_CLIP(0, color8p24::ONE - 1, src.g.raw);
    uint32_t c1b = XMC_CLIP(0, color8p24::ONE - 1, src.b.raw);
    uint32_t c1 = (color8p24::shiftCh(c1r, 20) & 0xF0000) |
                  (color8p24::shiftCh(c1g, 12) & 0xF00) |
                  (color8p24::shiftCh(c1b, 4) & 0xF);
    uint32_t r = c1 * a1 + c2 * a2;
    return ((r >> 12) & 0xF00) | ((r >> 8) & 0x0F0) | ((r >> 4) & 0x00F);
  }
}

static XMC_INLINE uint16_t add8p24To565(uint16_t dest, color8p24 src) {
  int32_t a1 = src.a.raw;
  if (a1 <= 0) {
    return dest;
  } else {
    int32_t r1 = src.r.raw >> (color8p24::PRECISION - 5);
    int32_t g1 = src.g.raw >> (color8p24::PRECISION - 6);
    int32_t b1 = src.b.raw >> (color8p24::PRECISION - 5);
    if (a1 < color8p24::ONE) {
      a1 = color8p24::shiftCh(a1, 6);
      r1 = (r1 * a1) >> 6;
      g1 = (g1 * a1) >> 6;
      b1 = (b1 * a1) >> 6;
    }
    int r2, g2, b2;
    unpack565(dest, &r2, &g2, &b2);
    return clipPack565(r1 + r2, g1 + g2, b1 + b2);
  }
}

static XMC_INLINE uint16_t add8p24To444(uint16_t dest, color8p24 src) {
  int32_t a1 = src.a.raw;
  if (a1 <= 0) {
    return dest;
  } else {
    int32_t r1 = src.r.raw >> (color8p24::PRECISION - 4);
    int32_t g1 = src.g.raw >> (color8p24::PRECISION - 4);
    int32_t b1 = src.b.raw >> (color8p24::PRECISION - 4);
    if (a1 < color8p24::ONE) {
      a1 = color8p24::shiftCh(a1, 4);
      r1 = (r1 * a1) >> 4;
      g1 = (g1 * a1) >> 4;
      b1 = (b1 * a1) >> 4;
    }
    int r2, g2, b2;
    unpack444(dest, &r2, &g2, &b2);
    return clipPack444(r1 + r2, g1 + g2, b1 + b2);
  }
}

}  // namespace xmc

#endif
