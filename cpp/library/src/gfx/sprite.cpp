#include "xmc/gfx2d/sprite.hpp"
#include "xmc/display.hpp"

namespace xmc {

XmcStatus startTransferToDisplay(Sprite sprite, int dx, int dy) {
  XMC_ERR_RET(display::setWindow(dx, dy, sprite->width, sprite->height));
  XMC_ERR_RET(display::writePixelsStart(
      sprite->linePtr(0), sprite->stride * sprite->height, false));
  return XMC_OK;
}

XmcStatus completeTransferToDisplay() { return display::writePixelsComplete(); }

}  // namespace xmc
