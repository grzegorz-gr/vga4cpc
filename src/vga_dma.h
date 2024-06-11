#ifndef __VGA_DMA_H__
#define __VGA_DMA_H__

void setup_vga_dma(uint8_t *screen, bool is_50hz);
void start_vga_dma();

#endif