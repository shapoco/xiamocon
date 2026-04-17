#include "xmc/gfx3d/worker3d.hpp"

namespace xmc {

template <bool BLEND, bool TEXTURE_4444, bool COLOR_TEXTURE,
          bool GOURAUD_SHADING, bool OUTPUT_444>
void renderTrapezoid3D(WorkerArgs3D &args, const Trapezoid3D &trap) {
  struct {
    int32_t zTestOffset : 20;
    bool blendAdd : 1;
    bool zUpdate : 1;
    bool written : 1;
  } flags;
  flags.zTestOffset = args.zTestOffset;
  flags.blendAdd = (args.blendMode == BlendMode::ADD);
  flags.zUpdate = hasFlag(args.renderFlags, RenderFlags3D::Z_UPDATE);
  flags.written = false;

  EdgeScanVars accumL = trap.topLeft;
  EdgeScanVars accumR = trap.topRight;

  for (int iy = trap.yTop; iy < trap.yBottom; iy++) {
    int32_t xL = accumL.x.roundToInt();
    int32_t xR = accumR.x.roundToInt();
    int32_t hSpan = xR - xL;
    int32_t xOffset = 0;

    if (xL < args.xMin) {
      xOffset = args.xMin - xL;
      xL = args.xMin;
    }
    if (xR > args.xMax) xR = args.xMax;

    if (xL < xR) {
      EdgeScanVars accumH = accumL;
      EdgeScanVars stepH = EdgeScanVars::subDiv(accumL, accumR, hSpan,
                                                GOURAUD_SHADING, COLOR_TEXTURE);

      if (xOffset > 0) {
        accumH.step(stepH, xOffset, GOURAUD_SHADING, COLOR_TEXTURE);
      }

      uint16_t *texData = (uint16_t *)args.textureData;
      Depth3D *depth = args.depthBuff + iy * args.depthStride + xL;
      RasterScan444 cPtr444((uint8_t *)args.target->linePtr(iy), xL);
      uint16_t *out565 = (uint16_t *)args.target->linePtr(iy) + xL;

      for (int i = xR - xL; i > 0; i--) {
        int32_t z = accumH.z.floorToInt();
        flags.written = 1;
        Depth3D zOrig = *depth;
        if (z + flags.zTestOffset < zOrig) {
          if (GOURAUD_SHADING || BLEND) {
            color8p24 c = accumH.c;
            if (COLOR_TEXTURE) {
              uint32_t uv = accumH.v.floorToInt() & args.vMask;
              uv *= args.textureStride;
              uv += accumH.u.floorToInt() & args.uMask;
              if (TEXTURE_4444) {
                c.multSelf4444(texData[uv]);
              } else {
                c.multSelf565(texData[uv]);
              }
            }
            if (BLEND) {
              if (c.a.raw > 0) {
                if (flags.blendAdd) {
                  if (OUTPUT_444) {
                    cPtr444.push444(add8p24To444(cPtr444.peek(), c));
                  } else {
                    uint16_t dst565 = *out565;
                    *out565 = add8p24To565(dst565, c);
                  }
                } else {
                  if (OUTPUT_444) {
                    cPtr444.push444(blend8p24To444(cPtr444.peek(), c));
                  } else {
                    uint16_t dst565 = *out565;
                    *out565 = blend8p24To565(dst565, c);
                  }
                }
                if (flags.zUpdate) *depth = z;
                flags.written = 0;
              }
            } else {
              if (OUTPUT_444) {
                cPtr444.push444(c.to444());
              } else {
                *out565 = c.to565();
              }
              if (flags.zUpdate) *depth = z;
              flags.written = 0;
            }
          } else {
            uint16_t c;
            if (COLOR_TEXTURE) {
              uint32_t uv = accumH.v.floorToInt() & args.vMask;
              uv *= args.textureStride;
              uv += accumH.u.floorToInt() & args.uMask;
              c = texData[uv];
              if (!TEXTURE_4444 && OUTPUT_444) {
                c = convert565To444(c);
              } else if (TEXTURE_4444 && !OUTPUT_444) {
                c = convert444To565(c);
              }
            } else {
              if (OUTPUT_444) {
                c = accumH.c.to444();
              } else {
                c = accumH.c.to565();
              }
            }
            if (OUTPUT_444) {
              cPtr444.push444(c);
            } else {
              *out565 = c;
            }
            if (flags.zUpdate) *depth = z;
            flags.written = 0;
          }
        }
        depth++;
        out565++;
        if (OUTPUT_444) {
          if (flags.written) {
            cPtr444.skip();
          }
        }
        accumH.step(stepH, 1, GOURAUD_SHADING, COLOR_TEXTURE);
      }
    }

    accumL.step(trap.stepLeft, 1, GOURAUD_SHADING, COLOR_TEXTURE);
    accumR.step(trap.stepRight, 1, GOURAUD_SHADING, COLOR_TEXTURE);
  }
}

void renderTrapezoid3D(WorkerArgs3D &args, const Trapezoid3D &trap,
                       uint32_t mode) {
#define XMC_TRAPEZOID(aBlend, tex4444, colorTex, gouraud, out444) \
  renderTrapezoid3D<aBlend, tex4444, colorTex, gouraud, out444>(args, trap)

  switch (mode) {
    case 0: XMC_TRAPEZOID(false, false, false, false, false); break;
    case 1: XMC_TRAPEZOID(false, false, false, false, true); break;
    case 2: XMC_TRAPEZOID(false, false, false, true, false); break;
    case 3: XMC_TRAPEZOID(false, false, false, true, true); break;
    case 4: XMC_TRAPEZOID(false, false, true, false, false); break;
    case 5: XMC_TRAPEZOID(false, false, true, false, true); break;
    case 6: XMC_TRAPEZOID(false, false, true, true, false); break;
    case 7: XMC_TRAPEZOID(false, false, true, true, true); break;
    case 8: XMC_TRAPEZOID(false, false, false, false, false); break;
    case 9: XMC_TRAPEZOID(false, false, false, false, true); break;
    case 10: XMC_TRAPEZOID(false, false, false, true, false); break;
    case 11: XMC_TRAPEZOID(false, false, false, true, true); break;
    case 12: XMC_TRAPEZOID(false, true, true, false, false); break;
    case 13: XMC_TRAPEZOID(false, true, true, false, true); break;
    case 14: XMC_TRAPEZOID(false, true, true, true, false); break;
    case 15: XMC_TRAPEZOID(false, true, true, true, true); break;
    case 16: XMC_TRAPEZOID(true, false, false, true, false); break;
    case 17: XMC_TRAPEZOID(true, false, false, true, true); break;
    case 18: XMC_TRAPEZOID(true, false, false, true, false); break;
    case 19: XMC_TRAPEZOID(true, false, false, true, true); break;
    case 20: XMC_TRAPEZOID(true, false, true, true, false); break;
    case 21: XMC_TRAPEZOID(true, false, true, true, true); break;
    case 22: XMC_TRAPEZOID(true, false, true, true, false); break;
    case 23: XMC_TRAPEZOID(true, false, true, true, true); break;
    case 24: XMC_TRAPEZOID(true, false, false, true, false); break;
    case 25: XMC_TRAPEZOID(true, false, false, true, true); break;
    case 26: XMC_TRAPEZOID(true, false, false, true, false); break;
    case 27: XMC_TRAPEZOID(true, false, false, true, true); break;
    case 28: XMC_TRAPEZOID(true, true, true, true, false); break;
    case 29: XMC_TRAPEZOID(true, true, true, true, true); break;
    case 30: XMC_TRAPEZOID(true, true, true, true, false); break;
    case 31: XMC_TRAPEZOID(true, true, true, true, true); break;
    default: break;
  }

#undef XMC_TRAPEZOID
}

void Worker3D::beginPrimitive(const WorkerArgs3D &args) {
  this->args = args;
  this->full = false;
  this->fifoWrPtr = this->fifoRdPtr;
}

void Worker3D::push(const Trapezoid3D &trap) {
  while (full) {
    tightLoopContents();
  }
  int wp = fifoWrPtr;
  fifo[wp] = trap;
  wp = (wp + 1) % FIFO_DEPTH;
  full = (wp == fifoRdPtr);
  fifoWrPtr = wp;
}

void Worker3D::endPrimitive() {
  while (full) {
    tightLoopContents();
  }
}

void Worker3D::process() {
  if (fifoRdPtr == fifoWrPtr && !full) {
    return;
  }

  uint32_t mode = getTrapezoidRenderMode(this->args);

  int rp = fifoRdPtr;
  while (rp != fifoWrPtr || full) {
    Trapezoid3D &trap = fifo[rp];

    renderTrapezoid3D(this->args, trap, mode);

    rp = (rp + 1) % FIFO_DEPTH;
    full = false;
  }
}

}  // namespace xmc
