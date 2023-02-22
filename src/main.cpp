#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include "ws2811/ws2811.h"
#include <signal.h>

#define LED_COUNT 10
#define CAPTURE_CARD_ID 0
#define GPIO_PIN 18
#define DMA 10
#define RGB_ORDER WS2811_STRIP_GRB

using namespace std;
using namespace cv;

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

int main(int argc, char **argv) {
    signal(SIGKILL,sig_kill_handler);
    ws2811_init(&led_strip);
    Mat frame;
    VideoCapture rca;
    rca.open(CAPTURE_CARD_ID, CAP_ANY);
    if (!rca.isOpened()) {
        fprintf(stderr, "Unable to open device ID: %d", CAPTURE_CARD_ID);
        return 1;
    }
    rca.read(frame);
    const unsigned short rows = frame.rows;
    const unsigned short cols = frame.cols;
    NeoPixel ambilight(LED_COUNT);
    int skip = ((cols * 2) + (rows * 2)) / LED_COUNT;
    int lengths = LED_COUNT / 4;

    unsigned char *temp;
    while (1) {
        if (frame.empty()) {
            fprintf(stderr, "missing frame");
            continue;
        }
        for (int i = skip; i < cols; i += skip) {
            temp = frame.ptr<unsigned char>(0, i);
            ambilight.setPixelColor(i - 4, temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(rows - 1, i);
            ambilight.setPixelColor((lengths * 2) + (i - 4), temp[0], temp[1], temp[2]);
        }
        for (int i = skip; i < rows; i += skip) {
            temp = frame.ptr<unsigned char>(i, 0);
            ambilight.setPixelColor(lengths + (i - 4), temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(i, cols - 1);
            ambilight.setPixelColor((lengths * 3) + (i - 4), temp[0], temp[1], temp[2]);
        }
        ambilight.clear();
        rca.read(frame);

    }

}