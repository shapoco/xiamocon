#include "xmc/flash.hpp"

#include <esp_partition.h>
#include <spi_flash_mmap.h>

namespace xmc::flash {

static const esp_partition_t *inesPartition = nullptr;

XmcStatus init() {
  inesPartition = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs");
  if (!inesPartition) {
    XMC_ERR_RET(XMC_ERR_FLASH_INIT_FAILED);
  }
  return XMC_OK;
}

void deinit() {}

size_t getSectorSize() { return SPI_FLASH_SEC_SIZE; }

void getRange(size_t *base, size_t *size) {
  if (base) *base = 0;
  if (size) *size = inesPartition->size;
}

XmcStatus write(uint32_t offset, const void *data, size_t size) {
  esp_err_t espErr;
  size_t sectorOffset = offset / SPI_FLASH_SEC_SIZE * SPI_FLASH_SEC_SIZE;
  size_t eraseSize = (offset + size - sectorOffset + SPI_FLASH_SEC_SIZE - 1) /
                     SPI_FLASH_SEC_SIZE * SPI_FLASH_SEC_SIZE;
  espErr = esp_partition_erase_range(inesPartition, sectorOffset, eraseSize);
  if (espErr != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_FLASH_ERASE_FAILED);
  }
  espErr = esp_partition_write(inesPartition, offset, data, size);
  if (espErr != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_FLASH_WRITE_FAILED);
  }
  return XMC_OK;
}

XmcStatus mmap(uint32_t offset, size_t size, void **handle,
               const uint8_t **outPtr) {
  spi_flash_mmap_handle_t *mmapHandle = new spi_flash_mmap_handle_t();
  esp_err_t espErr =
      esp_partition_mmap(inesPartition, offset, size, ESP_PARTITION_MMAP_DATA,
                         (const void **)outPtr, mmapHandle);
  if (espErr != ESP_OK) {
    *handle = nullptr;
    *outPtr = nullptr;
    XMC_ERR_RET(XMC_ERR_FLASH_MMAP_FAILED);
  }

  *handle = mmapHandle;
  return XMC_OK;
}

void munmap(void *handle) {
  if (!handle) return;
  spi_flash_mmap_handle_t mmapHandle = *(spi_flash_mmap_handle_t *)handle;
  spi_flash_munmap(mmapHandle);
  delete (spi_flash_mmap_handle_t *)handle;
}

}  // namespace xmc::flash
