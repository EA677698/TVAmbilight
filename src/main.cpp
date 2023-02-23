#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "ws2811/ws2811.h"
#include <signal.h>
#include <unistd.h>

#define LED_COUNT 37
#define GPIO_PIN 18
#define DMA 10
#define RGB_ORDER WS2811_STRIP_GRB
#define DEFAULT_REFRESH_RATE 60

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

void sig_kill_handler(int signum) {
    printf("\nProcess closing...\n");
    for(int i = 0; i<LED_COUNT; i++){
        setPixelColorRGB(i,0,0,0);
    }
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

int main(int argc, char **argv) {
    if(argc < 2){
        fprintf(stderr,"Invalid arguments\n"
                       "Usage: ./TVAmbilight [Video device ID] [Optional: refresh rate, default = 60]");
        return 0;
    }
    char* deviceID = *(argv+1);
    int refresh_rate;

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
    if(argc < 3){
        refresh_rate = 1000/DEFAULT_REFRESH_RATE;
        rca.set(CAP_PROP_FPS,DEFAULT_REFRESH_RATE);
    } else{
        refresh_rate = 1000/stoi(*(argv+2));
        rca.set(CAP_PROP_FPS,stoi(*(argv+2)));
    }

    rca.set(CAP_PROP_FOURCC,VideoWriter::fourcc('Y','U','Y','V'));
    rca.read(frame);
    const unsigned short rows = frame.rows;
    const unsigned short cols = frame.cols;
    int skip = ((cols * rows) - ((cols - 2) * (rows - 2))) / LED_COUNT;
    int lengths = LED_COUNT / 4;
    char attempts = 0;
    unsigned char *temp;
    int pixel;
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
        pixel = skip;
        for (int i = 0; i < lengths; i++) {
            temp = frame.ptr<unsigned char>(0, skip);
            setPixelColorRGB(i, temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(rows - 1, skip);
            setPixelColorRGB((lengths * 2) + i, temp[0], temp[1], temp[2]);
            pixel += skip;
        }
        pixel = skip;
        for (int i = 0; i < lengths; i++) {
            temp = frame.ptr<unsigned char>(skip, 0);
            setPixelColorRGB(lengths + i, temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(skip, cols - 1);
            setPixelColorRGB((lengths * 3) + i, temp[0], temp[1], temp[2]);
            pixel += skip;
        }
        ws2811_render(&led_strip);
        rca.read(frame);
        waitKey(refresh_rate);

    }

}