#include "xmc/dma_irq.hpp"

#include <hardware/dma.h>
#include <stddef.h>

namespace xmc::dma {

struct HandlerEntry {
  IrqHandlerCb handlerFast;
  IrqHandlerCb handlerSlow;
  void *context;
};

HandlerEntry contexts[16] = {0};

void registerIrqHandler(int dmaCh, IrqHandlerCb handlerFast,
                        IrqHandlerCb handlerSlow, void *context) {
  contexts[dmaCh].handlerFast = handlerFast;
  contexts[dmaCh].handlerSlow = handlerSlow;
  contexts[dmaCh].context = context;
}

void unregisterIrqHandler(int dmaCh) { contexts[dmaCh] = (HandlerEntry){0}; }

}  // namespace xmc::dma

extern "C" {

using namespace xmc::dma;

void xmcDmaIrqHandler(void) {
  uint32_t ints0 = dma_hw->ints0;
  uint32_t tmp = ints0;

  int dmaCh;
  do {
    dmaCh = __builtin_ctz(tmp);
    if (dmaCh >= 16) {
      break;
    }
    tmp &= ~(1u << dmaCh);

    HandlerEntry *entry = &contexts[dmaCh];
    if (entry->handlerFast) {
      entry->handlerFast(entry->context);
    }
  } while (tmp);

  tmp = ints0;
  do {
    dmaCh = __builtin_ctz(tmp);
    if (dmaCh >= 16) {
      break;
    }
    tmp &= ~(1u << dmaCh);

    HandlerEntry *entry = &contexts[dmaCh];
    if (entry->handlerSlow) {
      entry->handlerSlow(entry->context);
    }
  } while (tmp);

  dma_hw->ints0 = ints0;
}
}