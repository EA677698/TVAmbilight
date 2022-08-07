//
// Created by eafc6 on 7/31/2022.
//

#include <ws2811.h>
#include <stdio.h>
#include <signal.h>

#define TARGET_FREQ WS2811_TARGET_FREQ
#define LED_COUNT 300
#define GPIO_PIN 18
#define DMA 10
#define RGB_ORDER WS2811_STRIP_GRB
#define TV_BOTTOM_LEFT_INDEX tv_layout[0]
#define TV_LEFT_INDEX tv_layout[1]
#define TV_TOP_INDEX tv_layout[2]
#define TV_RIGHT_INDEX tv_layout[3]
#define TV_BOTTOM_RIGHT_INDEX tv_layout[4]

ws2811_t led_strip =
        {
        .freq = WS2811_TARGET_FREQ,
        .dmanum = DMA,
        .channel =
                {
                [0] =
                        {
                        .gpionum = GPIO_PIN,
                        .invert = 0,
                        .count = LED_COUNT,
                        .strip_type = RGB_ORDER,
                        .brightness = 255,
                        },
                [1] =
                        {
                        .gpionum = 0,
                        .invert = 0,
                        .count = 0,
                        .brightness = 0,
                        },
                },


        };

int tv_layout[] = {0,0,0,0,0};

void sig_kill_handler(int signum){
    printf("Process closing...");
    ws2811_fini(&led_strip);
    printf("Process ended");
}


void setBrightness(int b)
{
    if(b>0 && b<256) {
        led_strip.channel[0].brightness = b;
    } else{
        printf("Error: integer must be within range [0-255]");
    }
    return;
}

void setPixelColorRGB(int pixel, int r, int g, int b)
{
    if(b>0 && b<256) {
        if(pixel<LED_COUNT && pixel >=0) {
            led_strip.channel[0].leds[pixel] = (r << 16) | (g << 8) | b;
        } else{
            printf("Error: pixel must be within range [0-%d]",LED_COUNT);
        }
    } else{
        printf("Error: color integer must be within range [0-255]");
    }
    return;
}

int main(){
    ws2811_init(&led_strip);
    signal(SIGKILL,sig_kill_handler);
    return 0;
}