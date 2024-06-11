#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "hardware/pio.h"

// Pico GPIO pins assignment. They may change once I move from breadboard prototype to a real pcb.
#define FIRST_RGB_IN_PIN 0   // First of 8 inputs: blue, blue, green, green, red, red, csync, vsync
#define CSYNC_IN_PIN_PIO 6   // csync pin from Amstrad to PIO
#define FIRST_RGB_OUT_PIN 8  // First of 6 outputs: blue, blue, green, green, red, red
#define HSYNC_OUT_PIN 16     // VGA horizontal sync signal
#define VSYNC_OUT_PIN 17     // VGA vertical sync signal
#define VSYNC_GENERATOR_OUT_PIN 18  // Input vsync signal separated from csync. Goes to pin 7 and 22.
#define CSYNC_IN_PIN_HOST 21 // csync pin from Amstrad to host
#define VSYNC_IN_PIN_HOST 22 // vsync pin from VSYNC generating pio to host
#define VFREQ_SWITCH_PIN 26  // Switch between 576p50 (connected to GND) and 800x600p60 (not connected)

// Active width and height of CPC screen, and also the size of the copy that we keep in ram.
#define CPC_SCREEN_WIDTH 800
#define CPC_SCREEN_HEIGHT 296

// PIO for generating output (3 SMs - vsync, hsync and rgb)
#define VGA_PIO pio0
// PIO for reading input (2 SMs - one read rgb and one extracts vsync from csync)
#define CPC_PIO pio1

// State machines
#define VSYNC_GENERATOR_SM 0    // Reads CSync and generates VSync out of it. Output is used by both host and RGB_IN_SM
#define RGB_IN_SM 1             // Reads pixels from input and sends to host (rgbin.pio)
#define HSYNC_OUT_SM 0          // Generates output HSync signal (hsync_50.pio | hsync_60.pio)
#define VSYNC_OUT_SM 1          // Generates output VSync signal (hsync_50.pio | hsync_60.pio)
#define RGB_OUT_SM 2            // Generates output RGB signal (rgb_50.pio | rgb_60.pio)

// DMA channels
#define RGB_IN_CHANNEL 0            // for pixels from RGB_IN_SM to host
#define RGB_OUT_CONFIG_CHANNEL_A 1  // configures RGB_OUT_CHANNEL
#define RGB_OUT_CONFIG_CHANNEL_B 2  // configures RGB_OUT_CHANNEL
#define RGB_OUT_CONFIG_CHANNEL_C 3  // configures RGB_OUT_CHANNEL
#define RGB_OUT_CHANNEL 4           // for pixels from host to RGB_OUT_SM

#endif