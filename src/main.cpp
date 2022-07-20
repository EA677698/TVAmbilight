#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include "global.h"

using namespace std;

int send_data_to_arduino(FILE *arduino, char* data, char datatype) {
    char data_type[2];
    data_type[0] = '%';
    data_type[1] = datatype;
    for(int i = 0; data[i]; i++){
        if(fprintf(arduino, data_type, data[i])<0){
            return 1;
        }
        sleep(1);
    }
    return 0;
}

int connection_checker(FILE *arduino){
    char output[20];
    // @TODO check for acknowledgement and return based on that
    char acknowledgement[1];
    acknowledgement[0] = ACK;
    send_data_to_arduino(arduino, acknowledgement, 'c');
    printf(YES_CONNECTION);
    fread(&output,13,1,arduino);
    printf("%s\n",output);
    if(output!="ACK"){
        return 1;
    }
    printf("%s\n",output);

    return 0;
}


int main() {
    FILE *arduino;
    arduino = fopen("/dev/ttyS6","w+");
    if(!arduino){
        perror(NO_FILE);
        perror(NO_CONNECTION);
        exit(EXIT_FAILURE);
    }
    if(connection_checker(arduino)){
        perror(GENERIC_ERROR);
        exit(EXIT_FAILURE);
    }
    fclose(arduino);
    return 0;
}
