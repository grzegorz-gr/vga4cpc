#include <string.h>
#include "pico/types.h"
#include "test_screen.h"
#include "constants.h"


const char nosignal[7][107] = {
    "xxxxxxxxxxxx  xxxxxxxxxxxx      xxxxxxxxxxxx  xxxx  xxxxxxxxxxxx  xxxxxxxxxxxx     xxxxxx     xxxx",
    "xxxx    xxxx  xxxx    xxxx      xxxx    xxxx  xxxx  xxxx          xxxx    xxxx    xxxxxxxx    xxxx",
    "xxxx    xxxx  xxxx    xxxx      xxxx          xxxx  xxxx          xxxx    xxxx   xxxx  xxxx   xxxx",
    "xxxx    xxxx  xxxx    xxxx      xxxxxxxxxxxx  xxxx  xxxx  xxxxxx  xxxx    xxxx  xxxxxxxxxxxx  xxxx",
    "xxxx    xxxx  xxxx    xxxx              xxxx  xxxx  xxxx    xxxx  xxxx    xxxx  xxxx    xxxx  xxxx", 
    "xxxx    xxxx  xxxx    xxxx      xxxx    xxxx  xxxx  xxxx    xxxx  xxxx    xxxx  xxxx    xxxx  xxxx",
    "xxxx    xxxx  xxxxxxxxxxxx      xxxxxxxxxxxx  xxxx  xxxxxxxxxxxx  xxxx    xxxx  xxxx    xxxx  xxxxxxxxxxxx"
};
        
static void draw_square(uint8_t *screen, int x, int y) {
    uint8_t square[5][10] = {
        { 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f },
        { 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f },
        { 0x3f, 0x3f, 0x00, 0x00, 0x3f, 0x3f, 0x00, 0x00, 0x3f, 0x3f },
        { 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f },
        { 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f }
    };
    for (int line = 0; line < 5; ++line) {
        memcpy(&(screen[CPC_SCREEN_WIDTH * (y + line) + x]), &(square[line][0]), 10);
    }
}

static void draw_palette(uint8_t *screen, int x, int y) {
    int brightness[3] = {0, 1, 3};
    int palette[27];
    for (int r = 0; r < 3; ++r)
        for (int g = 0; g < 3; ++g)
            for (int b = 0; b < 3; ++b) {
                int index = r*9 + g*3 + b;
                int r_value = brightness[r];
                int g_value = brightness[g];
                int b_value = brightness[b];
                int color = (r_value << 4) | (g_value << 2) | b_value;
                palette[index] = color;
            }

    for (int row = 0; row < 3; ++row)
        for (int line = row * 20 + y; line < (row + 1) * 20 + y; ++line)
            for (int col = 0; col < 9 * 40; ++col)
                screen[line * CPC_SCREEN_WIDTH + col + x] = palette[row * 9 + col/40];
}

static void draw_no_signal(uint8_t *screen, int x, int y) {
    for (int row = 0; row < 7; ++row)
        for (int col = 0; nosignal[row][col]; ++col)
            screen[(y + row) * CPC_SCREEN_WIDTH + x + col] = (nosignal[row][col] == 'x') ? 0x3f : 0x00;
}

static void draw_pattern(uint8_t *screen, int x, int y, uint8_t color1, uint8_t color2) {
    for (int row = y; row < y + 20; ++row) {
        int col = x;
        for (int i = 1; i <= 9; ++i) {
            for (int k = 0; k < 4; ++k) {
                for (int j = 0; j < i; ++j)
                    screen[CPC_SCREEN_WIDTH * row + col++] = color1;
                for (int j = 0; j < i; ++j)
                    screen[CPC_SCREEN_WIDTH * row + col++] = color2;
            }
        }
    }
}

void fill_test_screen(uint8_t *screen) {
    // Fill screen with black pixels
    memset(screen, 0, CPC_SCREEN_WIDTH * CPC_SCREEN_HEIGHT);
    // Draw Amstrad CPC palette on top
    draw_palette(screen, 220, 10);
    // Draw lines from thin to thick, using while black and grey.
    // When something is wrong with vga timing, the lines will look bad - flickering or blurred
    draw_pattern(screen, 220, 220, 0x3f, 0x00);
    draw_pattern(screen, 220, 240, 0x00, 0x15);
    draw_pattern(screen, 220, 260, 0x15, 0x3f);
    // Just "no signal" in the middle of screen
    draw_no_signal(screen, 343, 150);
    // Square in each corner to check if nothing is cut
    draw_square(screen, 1, 8);
    draw_square(screen, CPC_SCREEN_WIDTH - 11, 8);
    draw_square(screen, 1, CPC_SCREEN_HEIGHT - 6);
    draw_square(screen, CPC_SCREEN_WIDTH - 11, CPC_SCREEN_HEIGHT - 6);
}
