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

//LED struct
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
//        if(kill(getpid(),SIGINT)){
//            fprintf(stderr,"Error: Failed to send signal, forcefully quitting...\n");
//        }
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
        exit(1);
    }
    printf("Process gracefully closed\n");
    exit(0);
}

int main(int argc, char **argv) {
    if(argc < 2){
        fprintf(stderr,"Invalid arguments\n"
                       "Usage: ./TVAmbilight [Video device ID] [Optional: refresh rate, default = 60]\n");
        return 0;
    }
    char* deviceID = *(argv+1);
    int refresh_rate;
    signal(SIGINT, sig_kill_handler);
    //LED initialization
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
    int skip = (rows*2)+(cols*2) / LED_COUNT;
    int width,length;
    int *large;
    int *small;
    float ratio = (float)rows/(float)cols;
    if(cols > rows){
        large = &length;
        small = &width;

    } else{
        large = &width;
        small = &length;
    }
    *small = LED_COUNT*(1-ratio);
    *large = LED_COUNT*ratio;
    if((width+length) < LED_COUNT){
        *large += (LED_COUNT - (length+width));
    }
    printf("cols: %d, rows: %d, large: %d, small: %d",cols,rows,*large,*small);
    char attempts = 0;
    unsigned char *temp;
    int pixel;
    while (1) {
        if (frame.empty()) {
            fprintf(stderr, "Error: Missing frame\n");
            attempts++;
            //auto killer
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
        //sets appropriate colors to pixels
        for (int i = 0; i < (length/2)-1; i++) { // cols
            temp = frame.ptr<unsigned char>(0, pixel);
            setPixelColorRGB(i, temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(rows - 1, pixel);
            setPixelColorRGB(((length/2)+(width/2)) + i, temp[0], temp[1], temp[2]);
            pixel += skip;
            printf("LOOP 1: Pixels modified: %d, %d",i,((length/2)+(width/2)) + i);
        }
        pixel = skip;
        for (int i = 0; i < (width/2)-1; i++) { //rows
            temp = frame.ptr<unsigned char>(pixel, 0);
            setPixelColorRGB((length/2) + i, temp[0], temp[1], temp[2]);
            temp = frame.ptr<unsigned char>(pixel, cols - 1);
            setPixelColorRGB(length + (width/2) + i, temp[0], temp[1], temp[2]);
            pixel += skip;
            printf("LOOP 2: Pixels modified: %d, %d",(length/2) + i,length + (width/2) + i);
        }
        //updates LED strip
        ws2811_render(&led_strip);
        //retrieves next frame
        rca.read(frame);
        //stalls program to meet refresh rate requirement while still allowing openCV to get frames and not time out
        waitKey(refresh_rate);

    }

}