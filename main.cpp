//
// leds_ctrl
// version 1.0, Loic Andre (c), https://github.com/Loicandre
// Software to control 2 Serial LEDs with rpi_PIO for the CCM5_rack project, LED1 is Status, LED2 is Net
// GPL-2.0
// *******************************************************************************************************************
// Reference : 
// https://github.com/raspberrypi/utils/tree/master/piolib
// https://github.com/adafruit/Adafruit_Blinka_Raspberry_Pi5_Neopixel/blob/main/src/main.cpp
// *******************************************************************************************************************

#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>

#include "piolib.h"
#include "ws2812.pio.h"


#define IS_RGBW false
#define NUM_PIXELS 2


static PIO pio;
static int sm;
static int offset;
bool verbose = false;
static int brightness = 15;

static void free_pio(void) {
    if (!pio) {
        return;
    }
    if (offset <= 0) {
        pio_remove_program(pio, &ws2812_program, offset);
    };
    offset = -1;
    if (sm >= 0) {
        pio_sm_unclaim(pio, sm);
    }
    sm = -1;
    pio_close(pio);
    pio = nullptr;
}


static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}


void log_print(const char *format, ...) {
    if (!verbose) return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

int arg_get_int(const char * arg, int min, int max) {

    int value;
    char *endptr;
    value = strtol(arg, &endptr, 10);
    if ((*endptr != '\0') || (value < min) ||(value > max)) {
        fprintf(stderr, "Invalid number: %s\n", arg);
        free_pio();
        exit(EXIT_FAILURE);
    }
    return value;
}

uint32_t arg_get_color(char c) {

    uint32_t color;

    if(c >= 'a')
        c= c - 0x20; // set all letter in high case

    switch (c) {
        case 'W':
            color = urgb_u32(brightness, brightness, brightness);
            break;        
        case 'X':
            color = urgb_u32(0, 0, 0);
            break;
        case 'R':
            color = urgb_u32(brightness, 0, 0);
            break;
        case 'E':
            color = urgb_u32(brightness, 0, brightness/2);
            break;
        case 'M':
            color = urgb_u32(brightness, 0, brightness);
            break;
        case 'V':
            color = urgb_u32(brightness/2, 0, brightness);
            break;
        case 'B':
            color = urgb_u32(0, 0, brightness);
            break;
        case 'A':
            color = urgb_u32(0, brightness/2, brightness);
            break;
        case 'C':
            color = urgb_u32(0, brightness, brightness);
            break;
        case 'S':
            color = urgb_u32(0, brightness, brightness/2);
            break;
        case 'G':
            color = urgb_u32(0, brightness, 0);
            break;
        case 'H':
            color = urgb_u32(brightness/2, brightness, 0);
            break;
        case 'Y':
            color = urgb_u32(brightness, brightness, 0);
            break;
        case 'O':
            color = urgb_u32(brightness, brightness/2, 0);
            break;
        default :
            fprintf(stderr, "Invalid color : %s\n", c);
            fprintf(stderr, "Known color : [w]White, [x]Off, [r]Red, [e]Rose, [m]Magenta, [v]Violet, [b]Blue\n");
            fprintf(stderr, "    [a]Azure, [c]Cyan, [s]SpringGreen, [g]Green, [h]YelGreen [y]Yellow, [O]Orange\n");
            free_pio();
            exit(EXIT_FAILURE);
    }
    return color;
}

int main(int argc, char *argv[])
{
    uint32_t gpio=18;
    char led_stat_char=0;
    char led_net_char=0;
    uint32_t led_Stat=0;
    uint32_t led_Net=0;

    stdio_init_all();

    const char *optstring = "hvb:g:s:n:";
    static struct option long_options[] = {
        {"help",        no_argument,        NULL, 'h'},
        {"verbose",     no_argument,        NULL, 'v'},
        {"gpio",        required_argument,  NULL, 'g'},
        {"bright",      required_argument,  NULL, 'b'},
        {"stat",        required_argument,  NULL, 's'},
        {"net",         required_argument,  NULL, 'n'}, 
        {0, 0, 0, 0}                               // Fin du tableau des options
    };
    int c;
    // command
    while ((c = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
    
        switch (c) {
        case 'h':
            printf("leds_ctrl, version 1.0, Loic Andre (c)\n");
            printf("Software to control 2 Serial LEDs with rpi_PIO for the CCM5_rack project, LED1 is Status, LED2 is Net\n");
            printf("");
            printf("Arguments: \n");
            printf("\t-h        \t-help          \t print this help \n");
            printf("\t-v        \t-verbose       \t use verbose to print debug messages and informations \n");
            printf("\t-g [gpio] \t-gpio   [gpio] \t Change the LED control GPIO number. (default 18)\n");
            printf("\t-b [1-255]\t-bright [1-255]\t Set the LED brightness. (default 50)\n");
            printf("\t-s [color]\t-stat   [color]\t Set the Status LED color (default Off)\n");
            printf("\t-n [color]\t-net    [color]\t Set the Network LED color (default Off)\n");
            printf("\n");
            printf("Known color (not case sensitive, only first letter is detected):\n");
            printf("  [w]White, [x]Off, [r]Red, [e]Rose, [m]Magenta, [v]Violet, [b]Blue\n");
            printf("  [a]Azure, [c]Cyan, [s]SpringGreen, [g]Green, [h]YelGreen [y]Yellow, [O]Orange\n");
            printf("\n");
            printf("Exemple: $ leds_ctrl -s r -n B -g 18\n");
            break;

        case 'v':
            verbose = true;
            break;

        case 'g':
            gpio = arg_get_int(optarg, 0, 255);
            log_print("Use gpio: %s\n", optarg);
            break;
            
        case 'b':
            brightness = arg_get_int(optarg, 1, 255);
            log_print("Set brightness to: %s\n", optarg);
            break;
            
        case 's':
            led_stat_char = optarg[0];
            log_print("LED Status color to: %s\n", optarg);
            break;
            
        case 'n':
            led_net_char = optarg[0];
            log_print("LED Net color to: %s\n", optarg);
            break;

        case '?':
            return 1;
        }
    }

    pio = pio0;

    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &ws2812_program);

    log_print("PIO Init: SM=%d, offset=%d, gpio=%d\n", sm, offset, gpio);
    ws2812_program_init(pio, sm, offset, gpio, 800000, IS_RGBW);

    if( !led_stat_char && !led_net_char)
    {
        // don't set leds if no order for it (like Help..)
        free_pio();
        return 0;
    }

    led_Stat = arg_get_color(led_stat_char);
    led_Net = arg_get_color(led_net_char);
    log_print("led_Stat=0x%06x, led_Net=0x%06x\n", led_Stat, led_Net);
    put_pixel(led_Stat);
    put_pixel(led_Net);

    free_pio();
    return 0;
}
