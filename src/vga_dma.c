#include "pico/types.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "vga_dma.h"
#include "constants.h"


// WARNING: This shit is really weird !!!

// The transfer from internal copy of Amstrad screen to VGA works in 1024 steps (it must be a power of 2).
// Each step handles one line... yeah... I know... there are no 1024 lines on input or output, so here comes the trick:
// Input has 296 lines from CPC (312 total minus 16 lines vsync). Output needs 800 or 576 lines (depending on output mode).
// In 800 lines mode:
// * steps 1..592 - We send each line from Amstrad two times to VGA
// * steps 593..600 - We send empty black line to VGA
// * steps 601..1024 - We send one byte to a temporary variable
// In 576 lines mode:
// * steps 1..576 - We send each line from amstrad two times to VGA, but we skip first 8 lines.
//        (I think first 10 lines are always black, so it should be OK)
// * steps 577..1024 - We send one byte to a temporary variable


uint8_t *line_src[1024] __attribute__((aligned(4096)));     // Pointers to line that should be send in each of 1024 steps
void *line_dst[1024] __attribute__((aligned(4096)));        // Pointer to destination in each step
uint32_t line_size[1024] __attribute__((aligned(4096)));    // Size of data to be send in each step

uint32_t tmp;  // Used as dma destination in the steps when we don't want to send anything to VGA
uint8_t black_line[CPC_SCREEN_WIDTH] = {0};


void setup_vga_dma(uint8_t *screen, bool is_50hz) {
    int skip_lines = is_50hz ? 8 : 0;   // skip first 8 lines only for 576p 50hz. 
    int lines_to_copy = CPC_SCREEN_HEIGHT - skip_lines;
    int dst_screen_height = is_50hz ? 576 : 600;
 
    for (int i = 0; i < lines_to_copy * 2; ++i) {           // Real lines are being sent here
        line_dst[i] = &(VGA_PIO->txf[RGB_OUT_SM]);               // Data are send to PIO queue
        line_src[i] = &(screen[CPC_SCREEN_WIDTH * (i/2 + skip_lines)]);  // We send each line twice, skipping first 8 or 0 lines
        line_size[i] = CPC_SCREEN_WIDTH;                    // We send whole line each time
    }
    // If there are some available lines in destination (800x600 case), fill them black
    for (int i = lines_to_copy * 2; i < dst_screen_height; ++i) {
        line_dst[i] = &(VGA_PIO->txf[RGB_OUT_SM]);   // Data are send to PIO queue
        line_src[i] = black_line;               // We send black line
        line_size[i] = CPC_SCREEN_WIDTH;        // We send whole line each time
    }
    for (int i = dst_screen_height; i < 1024; ++i) {    // Fake steps just to make them 1024 in total
        line_src[i] = &(screen[0]);         // Send first byte in screen, but actually it can be anything
        line_dst[i] = &tmp;                 // Send it to temporary variable
        line_size[i] = 1;                   // Send just one byte (so temporary variable can be small). I tried to send 0 but does not work.
    }

    // There are 4 channels in total: one that sends one line (transfer channel), and three that configure it for each step.

    // First configuration channel. It transfers one write address from "line_dst" to transfer channel config.
    // In each step it moves to the next address in "line_dst" until sends 2^12=4096 bytes (1024 4 byte addresses)
    // After if step it triggers next configuration channel.
    dma_channel_config config_a = dma_channel_get_default_config(RGB_OUT_CONFIG_CHANNEL_A);
    channel_config_set_transfer_data_size(&config_a, DMA_SIZE_32);
    channel_config_set_read_increment(&config_a, true);
    channel_config_set_write_increment(&config_a, false);
    channel_config_set_chain_to(&config_a, RGB_OUT_CONFIG_CHANNEL_B);
    channel_config_set_ring(&config_a, false, 12);
    dma_channel_configure(
        RGB_OUT_CONFIG_CHANNEL_A,
        &config_a,
        &dma_hw->ch[RGB_OUT_CHANNEL].al3_write_addr,
        &(line_dst[0]),
        1,
        false
    );

    // Second configuration channel. It transfers number of transfers "line_size" to transfer channel config.
    // Triggers third configuration channel.
    dma_channel_config config_b = dma_channel_get_default_config(RGB_OUT_CONFIG_CHANNEL_B);
    channel_config_set_transfer_data_size(&config_b, DMA_SIZE_32);
    channel_config_set_read_increment(&config_b, true);
    channel_config_set_write_increment(&config_b, false);
    channel_config_set_chain_to(&config_b, RGB_OUT_CONFIG_CHANNEL_C);
    channel_config_set_ring(&config_b, false, 12);
    dma_channel_configure(
        RGB_OUT_CONFIG_CHANNEL_B,
        &config_b,
        &dma_hw->ch[RGB_OUT_CHANNEL].al3_transfer_count,
        &(line_size[0]),
        1,
        false
    );
    
    // Third configuration channel. It transfers source address from "line_src" to transfer channel config.
    // Triggers transfer channel.
    dma_channel_config config_c = dma_channel_get_default_config(RGB_OUT_CONFIG_CHANNEL_C);
    channel_config_set_transfer_data_size(&config_c, DMA_SIZE_32);
    channel_config_set_read_increment(&config_c, true);
    channel_config_set_write_increment(&config_c, false);
    channel_config_set_ring(&config_c, false, 12);

    dma_channel_configure(
        RGB_OUT_CONFIG_CHANNEL_C,
        &config_c,
        &dma_hw->ch[RGB_OUT_CHANNEL].al3_read_addr_trig,
        &(line_src[0]),
        1,
        false
    );


    // Actual transfer channel. In each step, after it was configured, it sends one line to VGA pio.
    dma_channel_config config_main = dma_channel_get_default_config(RGB_OUT_CHANNEL);
    channel_config_set_transfer_data_size(&config_main, DMA_SIZE_8);
    channel_config_set_read_increment(&config_main, true);
    channel_config_set_write_increment(&config_main, false);
    channel_config_set_dreq(&config_main, DREQ_PIO0_TX2);
    channel_config_set_chain_to(&config_main, RGB_OUT_CONFIG_CHANNEL_A);

    dma_channel_configure(
        RGB_OUT_CHANNEL,
        &config_main,
        &(VGA_PIO->txf[RGB_OUT_SM]),
        &(screen[0]),
        CPC_SCREEN_WIDTH,
        false
    );
}

void start_vga_dma() {
    dma_start_channel_mask((1u << RGB_OUT_CONFIG_CHANNEL_A));
}
