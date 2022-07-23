#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <termios.h>
#include <cstdlib>
#include "global.h"

using namespace std;

int send_data_to_arduino(int &arduino, char *data, char datatype, int size) {
    char format[size + 1];
    format[0] = datatype;
    for(int i = 0; i<size; i++){
        format[i+1] = data[i];
    }
    printf("%s\n",format);
    int bytes = write(arduino, format, size+1);
    printf("%d bytes written",bytes);
    return 0;
}

int connection_checker(int &arduino){
    char buffer[256];
    // @TODO check for acknowledgement and return based on that
    char acknowledgement[1];
    acknowledgement[0] = ACK;
    //send_data_to_arduino(arduino, acknowledgement, 'c');
    printf(YES_CONNECTION);
    int result;
    while (!(result = read(arduino, &buffer, sizeof(buffer))));
    printf("%s\n",buffer);
    if(buffer!="ACK"){
        return 1;
    }
    printf("%s\n",buffer);

    return 0;
}


int main() {
    int arduino = open("/dev/ttyS6", O_RDWR);
    struct termios tty;
    if(arduino < 0){
        perror(NO_FILE);
        perror(NO_CONNECTION);
        exit(EXIT_FAILURE);
    }
    if(tcgetattr(arduino, &tty) != 0) {
        perror(GENERIC_ERROR);
    }
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    tty.c_cc[VTIME] = 100;
    tty.c_cc[VMIN] = 0;
    // Baud Rate Controls
    cfsetispeed(&tty, B1200);
    cfsetospeed(&tty, B1200);
    // End Baud Rate Controls
    if (tcsetattr(arduino, TCSANOW, &tty) != 0) {
        perror(GENERIC_ERROR);
    }
//    if(connection_checker(arduino)){
//        perror(GENERIC_ERROR);
//        exit(EXIT_FAILURE);
//    }
    char test[] = {'H','E','L','L','O','\n'};
    send_data_to_arduino(arduino,test,'c',6);
    close(arduino);
    return 0;
}
