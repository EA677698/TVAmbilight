# TVAmbilight

TVAmbilight is a simple ambient light system that can be used with a Raspberry Pi. It was created as an alternative to Hyperion and is still a work
in progress (with ongoing work to improve the user friendliness and interface). However, it's important to note that the system runs slowly on a Raspberry Pi Zero W, and
is therefore recommended to use a more powerful Raspberry Pi for best performance.

## Requirements

### Required libraries
* [OpenCV (any version should work)] (https://github.com/opencv/opencv)
* [rpi_ws281x] (https://github.com/jgarff/rpi_ws281x)

### Other requirements
* CMake Version 3.18 (or you can configure to work with older versions)
* Any Raspberry Pi (or anything with controllable pin headers)

## Configuration

Currently, there are no prompts through the terminal or any other GUI to make the configuration of this system more user-friendly. The only header that will be explained
here is the following:
```c++
#define CROPPED_LAYERS 20
```
This header controls how many borders are stripped from the video feed. You'd want to do this in the event the capture card introduces strange artifacts around
the border of the video source.

If you are wondering about how to configure the other headers, check out the [rpi_ws281x] (https://github.com/jgarff/rpi_ws281x) library's README.

## Build Instructions

The project has been configured to be built for ARM systems (Raspberry Pi Zero w). To change this remove the following lines:

```cmake
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
```

## Usage

To use TVAmbilight, follow these steps:

1. Make sure to run the program using administrative privileges.
2. Include the video source file.

Here's an example:

```bash
sudo ./TVAmbilight /dev/video0
```

To close the program, send a `SIGINT` signal to the program or press `Ctrl + C`

## Conclusion

Thank you for checking the project out. I will continue to develop it once I get a more powerful Raspberry Pi. If you want more information on how this project was made check
out my [website] (www.erickalanis.com). Feel free to contribute to the project, fork it, or send me any feedback for the project.
