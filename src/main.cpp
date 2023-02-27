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
#define CROPPED_LAYERS 20

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

int arraysize = LED_COUNT * sizeof(ws2811_led_t);

void sig_kill_handler(int signum) {
    printf("\nProcess closing...\n");
    memset(led_strip.channel[0].leds,0,arraysize);
    setBrightness(0);
    ws2811_render(&led_strip);
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
    signal(SIGINT, sig_kill_handler);
    //LED initialization
    ws2811_init(&led_strip);
    Mat frame;
    //rca.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    rca.open(deviceID, CAP_ANY);
    if (!rca.isOpened()) {
        fprintf(stderr, "Error: Unable to open device ID: %s\n", deviceID);
        if(kill(getpid(),SIGINT)){
            fprintf(stderr,"Error: Failed to send signal, forcefully quitting...\n");
            return 1;
        }
    }
    rca.read(frame);
    const unsigned short rows = frame.rows - CROPPED_LAYERS;
    const unsigned short cols = frame.cols - CROPPED_LAYERS;
    int skip = ((rows*2)+(cols*2)) / LED_COUNT;
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
    *small = (LED_COUNT*(1-ratio))/2;
    *large = (LED_COUNT*ratio)/2;
    if((2*(width+length)) < LED_COUNT){
        *large += (LED_COUNT - (2*(length+width)));
    }
    char attempts = 0;
    Vec3b temp;
    int pixel;
    int row2 = rows - 1 - CROPPED_LAYERS;
    int cols2 = cols - 1 - CROPPED_LAYERS;
    int widthlength = length+ width;
    int thirdsection = (2 * length) + width;
    while (1) {
        if (frame.empty()) {
            fprintf(stderr, "Error: Missing frame\n");
//            attempts++;
//            //auto killer
//            if(attempts>=32){
//                fprintf(stderr,"Error: Too many frame retrieval attempts failed! Exiting program...\n");
//                if(kill(getpid(),SIGINT)){
//                    fprintf(stderr,"Error: Failed to send signal, forcefully quitting...\n");
//                    return 1;
//                }
//            }
            continue;
        }
        attempts = 0;
        pixel = skip;
        //sets appropriate colors to pixels
        for (int i = 0; i < length - 1; i+=2) { // cols
            temp = frame.at<cv::Vec3b>(CROPPED_LAYERS, pixel);
            led_strip.channel[0].leds[i] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            temp = frame.at<cv::Vec3b>(row2, pixel);
            led_strip.channel[0].leds[widthlength + i] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            pixel += skip;
            temp = frame.at<cv::Vec3b>(CROPPED_LAYERS, pixel);
            led_strip.channel[0].leds[i + 1] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            temp = frame.at<cv::Vec3b>(row2, pixel);
            led_strip.channel[0].leds[widthlength + i + 1] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
        }
        pixel = skip;
        for (int i = 0; i < width-1; i += 2) { //rows
            temp = frame.at<cv::Vec3b>(pixel, CROPPED_LAYERS);
            led_strip.channel[0].leds[length + i] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            temp = frame.at<cv::Vec3b>(pixel, cols2);
            led_strip.channel[0].leds[thirdsection + i] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            pixel += skip;
            temp = frame.at<cv::Vec3b>(pixel, CROPPED_LAYERS);
            led_strip.channel[0].leds[length + i + 1] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            temp = frame.at<cv::Vec3b>(pixel, cols2);
            led_strip.channel[0].leds[thirdsection + i + 1] = (temp[2] << 16) | (temp[1] << 8) | temp[0];
            pixel += skip;
        }
        //updates LED strip
        if (ws2811_render(&led_strip) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed\n");
            break;
        }
        //retrieves next frame
        rca.read(frame);
    }

}