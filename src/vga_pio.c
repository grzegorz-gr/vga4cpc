#include "hardware/pio.h"
#include "hsync_50.pio.h"
#include "vsync_50.pio.h"
#include "rgb_50.pio.h"
#include "hsync_60.pio.h"
#include "vsync_60.pio.h"
#include "rgb_60.pio.h"
#include "vga_pio.h"
#include "constants.h"

// A couple of VGA related constants that can't be set in pio asm because they are too big,
// so they are being sent though queue:
#define V_ACTIVE_60_MINUS1 599  // Number of active lines in 800x600x60 mode minus 1
#define V_ACTIVE_50_MINUS1 575  // Number of active lines in 720x576x50 mode minus 1
#define RGB_ACTIVE_MINUS1 799   // Number of pixels per line (in every mode) minus 1
#define H_ACTIVE_FRONT_60_MINUS2 313  // Number of cpu cycles spent in active part of line and front porch minus 2 (800x600x60 mode)
#define H_ACTIVE_FRONT_50_MINUS2 181  // Number of cpu cycles spent in active part of line and front porch minus 2 (720x576x50 mode)


void setup_vga_pio(bool is_50hz) {
    uint hsync_offset = pio_add_program(VGA_PIO, is_50hz ? &hsync_50_program : &hsync_60_program);
    uint vsync_offset = pio_add_program(VGA_PIO, is_50hz ? &vsync_50_program : &vsync_60_program);
    uint rgb_offset = pio_add_program(VGA_PIO, is_50hz ? &rgb_50_program : &rgb_60_program);

    if (is_50hz) {
            hsync_50_program_init(VGA_PIO, HSYNC_OUT_SM, hsync_offset, HSYNC_OUT_PIN);
            vsync_50_program_init(VGA_PIO, VSYNC_OUT_SM, vsync_offset, VSYNC_OUT_PIN);
            rgb_50_program_init(VGA_PIO, RGB_OUT_SM, rgb_offset, 8);
    } else {
            hsync_60_program_init(VGA_PIO, HSYNC_OUT_SM, hsync_offset, HSYNC_OUT_PIN);
            vsync_60_program_init(VGA_PIO, VSYNC_OUT_SM, vsync_offset, VSYNC_OUT_PIN);
            rgb_60_program_init(VGA_PIO, RGB_OUT_SM, rgb_offset, 8);
    }

    pio_sm_put_blocking(VGA_PIO, HSYNC_OUT_SM, is_50hz ? H_ACTIVE_FRONT_50_MINUS2 : H_ACTIVE_FRONT_60_MINUS2);
    pio_sm_put_blocking(VGA_PIO, VSYNC_OUT_SM, is_50hz ? V_ACTIVE_50_MINUS1 : V_ACTIVE_60_MINUS1);
    pio_sm_put_blocking(VGA_PIO, RGB_OUT_SM, RGB_ACTIVE_MINUS1);
}

void start_vga_pio() {
    pio_enable_sm_mask_in_sync(VGA_PIO, ((1u << HSYNC_OUT_SM) | (1u << VSYNC_OUT_SM) | (1u << RGB_OUT_SM)));
}
