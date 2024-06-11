#include "cpc_dma.h"
#include "constants.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "test_screen.h"
#include <string.h>


// Ufortunately, the number of input lines is unpredictable and we can't just set up the dma once.
// We need a loop that reads frame line by line and restarts once VSYNC arrives.
// It works well with smaller number of lines like in Ultima Ratio or Prehistorik II loading screen.

void start_cpc_dma_loop(uint8_t *screen) {
    gpio_init(CSYNC_IN_PIN_HOST);
    gpio_set_dir(CSYNC_IN_PIN_HOST, GPIO_IN);
    gpio_init(VSYNC_IN_PIN_HOST);
    gpio_set_dir(VSYNC_IN_PIN_HOST, GPIO_IN);

    int next_line_idx = 0;
    
    while (1) {     // One frame is read in one iteration

        // We may be in the middle of frame, so wait for vsync to get active - beginning of new frame.
        bool test_screen_on = false;
        while (!gpio_get(VSYNC_IN_PIN_HOST));
        // Then let's wait to the end of vsync, so the frame actually starts
        while (gpio_get(VSYNC_IN_PIN_HOST));

        // Previous frame could be shorter so fill the remaining part of screen with black pixels
        memset (&(screen[CPC_SCREEN_WIDTH * next_line_idx]), 0, (CPC_SCREEN_HEIGHT - next_line_idx) * CPC_SCREEN_WIDTH);

        next_line_idx = 0;  // Start writing from line 0
        
        // Each iteration is one line. We repeat it until we reach maximum number of lines or until next vsync.
        while (!gpio_get(VSYNC_IN_PIN_HOST) && next_line_idx < CPC_SCREEN_HEIGHT) {

            // Wait until csync goes down, then up, so it means it is the start of a new line.
            // Unless it went down because of vsync, so we need to break the loop earlier.

            while (gpio_get(CSYNC_IN_PIN_HOST)) {
                if (gpio_get(VSYNC_IN_PIN_HOST) || next_line_idx >= CPC_SCREEN_HEIGHT) break;
            }
            while (!gpio_get(CSYNC_IN_PIN_HOST)) {
                if (gpio_get(VSYNC_IN_PIN_HOST) || next_line_idx >= CPC_SCREEN_HEIGHT) break;
            }
            
            // Then we just process the next line - set up dma and start it

            dma_channel_config c = dma_channel_get_default_config(RGB_IN_CHANNEL);
            channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
            channel_config_set_read_increment(&c, false);
            channel_config_set_write_increment(&c, true);
            channel_config_set_dreq(&c, pio_get_dreq(CPC_PIO, RGB_IN_SM, false)) ;
            dma_channel_configure(
                RGB_IN_CHANNEL,
                &c,
                &(screen[CPC_SCREEN_WIDTH * next_line_idx]),
                &CPC_PIO->rxf[RGB_IN_SM],
                CPC_SCREEN_WIDTH,
                true
            );

            ++next_line_idx;
        }
    }
}
