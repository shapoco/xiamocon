#ifndef XMC_GFX_PAINT_HPP
#define XMC_GFX_PAINT_HPP

#include "xmc/gfx2d/glyph_renderer.hpp"
#include "xmc/gfx2d/glyph_renderer_agfx.hpp"
#include "xmc/gfx2d/raster_scan.hpp"
#include "xmc/gfx2d/sprite.hpp"

#include <memory>

namespace xmc {

class Graphics2DClass {
 protected:
  Sprite target;
  GraphicsState2D state;

 public:
  Graphics2DClass() : target(nullptr) {
    state.clipRect = {0, 0, display::WIDTH, display::HEIGHT};
  }
  Graphics2DClass(const Sprite &target) : target(target) {
    state.clipRect = {0, 0, target->width, target->height};
  }

  void setTarget(Sprite &s);
  inline Sprite getTarget() const { return target; }
  void setClipRect(const Rect &rect);
  inline void setClipRect(int x, int y, int w, int h) {
    setClipRect(Rect{x, y, w, h});
  }
  void clearClipRect();
  Rect getBounds();
  GraphicsState2D getState() { return state; }
  void setState(const GraphicsState2D &s) { state = s; }

  inline void clear(RawColor color) {
    fillRect(state.clipRect.x, state.clipRect.y, state.clipRect.width,
             state.clipRect.height, color);
  }
  void fillRect(int x, int y, int w, int h, RawColor color);
  void drawRect(int x, int y, int w, int h, RawColor color);
  void fillSmokeRect(int x, int y, int w, int h, bool white = false);

  void drawImage(const Sprite &image, int dx, int dy, int w, int h, int sx,
                 int sy, const TextRenderArgs &tra);
  inline void drawImage(const Sprite &image, int dx, int dy, int w, int h,
                        int sx, int sy) {
    TextRenderArgs tra;
    tra.foreColor = 0xFFFF;
    tra.backColor = 0x0000;
    tra.flags = TextRenderFlags::DRAW_FORE | TextRenderFlags::DRAW_BACK;
    drawImage(image, dx, dy, w, h, sx, sy, tra);
  }

  inline void setFont(const GFXfont *font, int size) {
    setFont(font);
    setFontSize(size);
  }
  void setFont(const GFXfont *font);
  void setFontSize(int size);
  void setCursor(int x, int y);
  void setTextColor(RawColor fg);
  void setTextColor(RawColor fg, RawColor bg);
  void drawString(const char *str);
  int drawChar(GlyphRenderer &renderer, int x, int y, int code);
};

using Graphics2D = std::shared_ptr<Graphics2DClass>;

static inline Graphics2D createGraphics2D() {
  return std::make_shared<Graphics2DClass>();
}

static inline Graphics2D createGraphics2D(Sprite target) {
  return std::make_shared<Graphics2DClass>(target);
}

}  // namespace xmc

#endif
