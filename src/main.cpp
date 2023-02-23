#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include "ws2811/ws2811.h"
#include <signal.h>
#include <unistd.h>

#define LED_COUNT 37
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

VideoCapture rca;

void sig_kill_handler(int signum) {
    printf("\nProcess closing...\n");
    ws2811_fini(&led_strip);
    rca.release();
    if(!rca.isOpened()){
        printf("Video stream closed successfully!\n");
    } else{
        fprintf(stderr,"Error: Failed to close video stream!\n");
    }
    printf("Process gracefully closed\n");
    exit(0);
}

void setBrightness(unsigned short b) {
    led_strip.channel[0].brightness = b;
}

void setPixelColorRGB(int pixel, unsigned short r, unsigned short g, unsigned short b) {
    if (pixel < LED_COUNT && pixel >= 0) {
        led_strip.channel[0].leds[pixel] = (r << 16) | (g << 8) | b;
    } else {
        fprintf(stderr,"Error: pixel must be within range [0-%d]\n"
                       "Given pixel: %d\n", LED_COUNT, pixel);
    }
}

int main(int argc, char **argv) {
    char* deviceID = *(argv+1);
    signal(SIGINT, sig_kill_handler);
    ws2811_init(&led_strip);
    Mat frame;
    rca.open(deviceID, CAP_ANY);
    if (!rca.isOpened()) {
        fprintf(stderr, "Error: Unable to open device ID: %s\n", deviceID);
        if(kill(getpid(),SIGINT)){
            fprintf(stderr,"Error: Failed to send signal, forcefully quitting...\n");
            return 1;
        }
    }
    rca.read(frame);
    const unsigned short rows = frame.rows;
    const unsigned short cols = frame.cols;
    int skip = ((cols * rows) - ((cols - 2) * (rows - 2))) / LED_COUNT;
    int lengths = LED_COUNT / 4;
    char attempts = 0;
    unsigned char *temp;
    while (1) {
        if (frame.empty()) {
            fprintf(stderr, "Error: Missing frame\n");
            attempts++;
            if(attempts>=32){
                fprintf(stderr,"Error: Too many frame retrieval attempts failed! Exiting program...\n");
                if(kill(getpid(),SIGINT)){
                    fprintf(stderr,"Error: Failed to send signal, forcefully quitting...\n");
                    return 1;
                }
            }
            continue;
        }
        attempts = 0;
        for (int i = skip; i < cols; i += skip) {
            temp = frame.ptr<unsigned char>(0, i);
            setPixelColorRGB(i - 4, temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(rows - 1, i);
            setPixelColorRGB((lengths * 2) + (i - 4), temp[0], temp[1], temp[2]);
        }
        for (int i = skip; i < rows; i += skip) {
            temp = frame.ptr<unsigned char>(i, 0);
            setPixelColorRGB(lengths + (i - 4), temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(i, cols - 1);
            setPixelColorRGB((lengths * 3) + (i - 4), temp[0], temp[1], temp[2]);
        }
        ws2811_render(&led_strip);
        rca.read(frame);

    }

}