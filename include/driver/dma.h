
#ifndef __DMA_H__
#define __DMA_H__

#include <types.h>
#include <stdint.h>
#include <stddef.h>

/*
 * enum dma_direction - dma transfer direction indicator
 * @DMA_MEM_TO_MEM: Memcpy mode
 * @DMA_MEM_TO_DEV: From Memory to Device
 * @DMA_DEV_TO_MEM: From Device to Memory
 * @DMA_DEV_TO_DEV: From Device to Device
 */
enum dma_direction {
	DMA_MEM_TO_MEM,
	DMA_MEM_TO_DEV,
	DMA_DEV_TO_MEM,
	DMA_DEV_TO_DEV,
};

#define BIT(n)                  (1 << n)

#define DMA_SUPPORTS_MEM_TO_MEM BIT(0)
#define DMA_SUPPORTS_MEM_TO_DEV BIT(1)
#define DMA_SUPPORTS_DEV_TO_MEM BIT(2)
#define DMA_SUPPORTS_DEV_TO_DEV BIT(3)
#define DMA_SUPPORTS_SPI        BIT(8)


struct dma_ops {
	int   (*transfer)(void *dst, void *src, size_t len, int direction);
	void* (*dma_addr_get)(void *addr, uint32_t flag);
};

extern struct dma_ops *dma_ops_get(void);
extern int    dma_memcpy(void *dst, void *src, size_t len);
extern int    dma_xfer(void *dest,void *src, size_t len, int direction);



#endif

