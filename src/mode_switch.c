#include "hardware/gpio.h"
#include "pico/types.h"
#include "constants.h"
#include "mode_switch.h"

static void on_freq_changed(uint gpio, uint32_t events) {
    *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;  // Software reset
}

void setup_vfreq_and_switch() {
    gpio_init(VFREQ_SWITCH_PIN);
    gpio_set_dir(VFREQ_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(VFREQ_SWITCH_PIN);     // We set pull up, so we don't neet a pull up resistor. just a switch to GND.

    // Set callback, so if vfreq switch is switched (on to off or other way round) Pico is reset:
    gpio_set_irq_enabled_with_callback(26, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &on_freq_changed);
}

bool is_50hz() {
    // Return true if (pulled up) pin is not connected to GND.
    return gpio_get(VFREQ_SWITCH_PIN);
}