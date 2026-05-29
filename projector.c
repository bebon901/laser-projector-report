#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "image.c"

const uint RED_PIN_3 = 3;
const uint RED_PIN_2 = 2;
const uint RED_PIN_1 = 1;
const uint RED_PIN_0 = 0;

const uint GREEN_PIN_3 = 7;
const uint GREEN_PIN_2 = 6;
const uint GREEN_PIN_1 = 5;
const uint GREEN_PIN_0 = 4;

const uint BLUE_PIN_3 = 11;
const uint BLUE_PIN_2 = 10;
const uint BLUE_PIN_1 = 9;
const uint BLUE_PIN_0 = 8;

const uint HOZ_TRIGGER_PIN = 16;
const uint VERT_TRIGGER_PIN = 17;

uint HOZ_TRIG_COUNT = 0;
uint HOZ_TRIG_FREQ_COUNT = 0;
uint VERT_TRIG_COUNT = 0;

static uint64_t vert_last_trig_us = 0;
static uint64_t prev_frame_time = 1000;
static uint64_t last_trig_us = 0;
static uint64_t max_line_sleep_us = 100;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == HOZ_TRIGGER_PIN) {
        // Line time
        uint64_t now_us = to_us_since_boot(get_absolute_time());
        uint64_t line_time = now_us - last_trig_us;
        uint64_t unit_us = line_time / 100;

        // Get the line height - taken from the frame timers?
        int line_pos_y = (((now_us - vert_last_trig_us) * IMG_ROWS) / prev_frame_time);

        if (line_pos_y >= IMG_ROWS) line_pos_y = IMG_ROWS - 1;
        if (line_pos_y < 0) line_pos_y = 0;

        // Dot location proportional to the time since frame started?
        // About 16 units of rotation till the image should be drawn
        sleep_us(15 * unit_us);
        // sleep this much:
        // sleep_us(MIN((now_us - vert_last_trig_us) / prev_frame_time * max_line_sleep_us, max_line_sleep_us));
        
        // printf("%d", line_pos_y);
        for (int i = 0; i < IMG_COLS; i++) {
            int val = image_data[line_pos_y][i];
            // Use Bitwise AND (&)
            gpio_put_masked(0xFFF, val);
            //for(volatile int delay = 0; delay < 30; delay++);
            sleep_us(70 * unit_us / IMG_COLS);
        }
        // finished drawing image - turn back on for detection of next round.
        gpio_put(RED_PIN_1, 1);
        gpio_put(RED_PIN_2, 1);
        gpio_put(RED_PIN_0, 1);
        gpio_put(RED_PIN_3, 1);
            
        gpio_put(GREEN_PIN_0, 1);
        gpio_put(GREEN_PIN_1, 1);
        gpio_put(GREEN_PIN_2, 1);
        gpio_put(GREEN_PIN_3, 1);
            
        gpio_put(BLUE_PIN_0, 1);
        gpio_put(BLUE_PIN_1, 1);
        gpio_put(BLUE_PIN_2, 1);
        gpio_put(BLUE_PIN_3, 1);
        // Update the last trigger time
        last_trig_us = now_us;
        HOZ_TRIG_COUNT++;
        HOZ_TRIG_FREQ_COUNT++;
    };
    if (gpio == VERT_TRIGGER_PIN) {
        // Update the horizontal timestamp to enable vertical position sensing in horizontal stream.
        prev_frame_time = to_us_since_boot(get_absolute_time()) - vert_last_trig_us;
        vert_last_trig_us = to_us_since_boot(get_absolute_time());
        VERT_TRIG_COUNT++;
        HOZ_TRIG_COUNT = 0;
    }
}

int main() {
    stdio_init_all();

    // GPIO Initialization
    const uint output_pins[] = {
        RED_PIN_1, RED_PIN_2,
        RED_PIN_3, RED_PIN_0, 
        GREEN_PIN_3, GREEN_PIN_2, GREEN_PIN_1, GREEN_PIN_0,
        BLUE_PIN_3, BLUE_PIN_2, BLUE_PIN_1, BLUE_PIN_0
    };

    // Loop through the array to initialize each pin
    for (int i = 0; i < 12; i++) {
        gpio_init(output_pins[i]);
        gpio_set_dir(output_pins[i], GPIO_OUT);
        gpio_put(output_pins[i], 0); // Start with all LEDs off
    }

    // Initialize the input pins for the interrupts
    gpio_init(HOZ_TRIGGER_PIN);
    gpio_set_dir(HOZ_TRIGGER_PIN, GPIO_IN);
    gpio_pull_down(HOZ_TRIGGER_PIN);

    gpio_init(VERT_TRIGGER_PIN);
    gpio_set_dir(VERT_TRIGGER_PIN, GPIO_IN);
    gpio_pull_down(VERT_TRIGGER_PIN);

    gpio_set_irq_enabled_with_callback(HOZ_TRIGGER_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_set_irq_enabled(VERT_TRIGGER_PIN, GPIO_IRQ_EDGE_FALL, true);

    printf("✅ System ready. Waiting for a falling edge on GP%d...\n", HOZ_TRIGGER_PIN);

    gpio_put(RED_PIN_1, 1);
    gpio_put(RED_PIN_2, 1);
    gpio_put(RED_PIN_3, 1);
    gpio_put(RED_PIN_0, 1);
            
    gpio_put(GREEN_PIN_0, 1);
    gpio_put(GREEN_PIN_1, 1);
    gpio_put(GREEN_PIN_2, 1);
    gpio_put(GREEN_PIN_3, 1);

    gpio_put(BLUE_PIN_0, 1);
    gpio_put(BLUE_PIN_1, 1);
    gpio_put(BLUE_PIN_2, 1);
    gpio_put(BLUE_PIN_3, 1);

    // --- Main Loop ---
    while (1) {
        sleep_ms(1000);
        printf("Framerates are Vertical Refresh Rate: %d Hz, Horizontal Refresh Rate: %d Hz\n", VERT_TRIG_COUNT, HOZ_TRIG_FREQ_COUNT);
        printf("Previous frame time is: %llu us\n", prev_frame_time);
        HOZ_TRIG_FREQ_COUNT = 0;
        VERT_TRIG_COUNT = 0; 
    }

    return 0;
}