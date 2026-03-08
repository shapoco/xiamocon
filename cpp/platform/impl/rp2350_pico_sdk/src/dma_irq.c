#include "xmc/hw/dma_irq.h"

#include <hardware/dma.h>
#include <stddef.h>

typedef struct {
  xmc_dma_irq_handler_t handler;
  void *context;
} handler_entry_t;

handler_entry_t contexts[16] = {0};

void xmc_dma_register_irq_handler(int dma_ch, xmc_dma_irq_handler_t handler,
                                  void *context) {
  contexts[dma_ch].handler = handler;
  contexts[dma_ch].context = context;
}

void xmc_dma_unregister_irq_handler(int dma_ch) {
  contexts[dma_ch] = (handler_entry_t){0};
}

void xmc_dma_irq_handler(void) {
  uint32_t ints0 = dma_hw->ints0;
  uint32_t tmp = ints0;

  int dma_ch;
  do {
    dma_ch = __builtin_ctz(tmp);
    if (dma_ch >= 16) {
      break;
    }
    tmp &= ~(1u << dma_ch);

    handler_entry_t entry = contexts[dma_ch];
    if (entry.handler) {
      entry.handler(entry.context);
    }
  } while (tmp);

  dma_hw->ints0 = ints0;
}
