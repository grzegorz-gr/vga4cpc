#include "hardware/pio.h"
#include "rgbin.pio.h"
#include "vsyncgen.pio.h"
#include "cpc_pio.h"
#include "constants.h"

#define CPC_SCREEN_WIDTH_MINUS1 (CPC_SCREEN_WIDTH - 1)

void setup_cpc_pio() {
    // Setup and start PIO sm that reads RGB from Amstrad
    uint rgbin_offset = pio_add_program(CPC_PIO, &rgbin_program);
    rgbin_program_init(CPC_PIO, RGB_IN_SM, rgbin_offset, FIRST_RGB_IN_PIN);
    // We need to pass number of active pixels through queue, because the value is to big
    // to set it in pio asm.
    pio_sm_put_blocking(CPC_PIO, RGB_IN_SM, CPC_SCREEN_WIDTH_MINUS1);

    // Set up and start PIO sm that creates vsync from input csync.
    uint vsyncgen_offset = pio_add_program(CPC_PIO, &vsyncgen_program);
    vsyncgen_program_init(CPC_PIO, VSYNC_GENERATOR_SM, vsyncgen_offset, CSYNC_IN_PIN_PIO, VSYNC_GENERATOR_OUT_PIN);
}