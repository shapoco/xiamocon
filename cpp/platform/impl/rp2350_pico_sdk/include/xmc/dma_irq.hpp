/**
 * @file dma_irq.hpp
 * @brief DMA interrupt handling interface
 */

#ifndef XMC_HW_DMA_IRQ_HPP
#define XMC_HW_DMA_IRQ_HPP

#include "xmc/dma.hpp"

namespace xmc::dma {

typedef void (*IrqHandlerCb)(void *context);

void registerIrqHandler(int dmaCh, IrqHandlerCb handlerFast,
                        IrqHandlerCb handlerSlow, void *context);
void unregisterIrqHandler(int dmaCh);

}  // namespace xmc::dma

extern "C" {
void xmcDmaIrqHandler(void);
}

#endif
