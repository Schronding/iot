#include <stdio.h>
#include <stdlib.h>
#include "Utils.h"
#include "Statistics.h"

typedef struct{
    float * temp;
    float * humi; 
    float * ligh; 
    float std_dev;
    float mean;
} sensor;

int main(){
    int n = 100;
    sensor TSL2560; 
    sensor DHT22; 
    DHT22.temp = (float * )malloc(n * sizeof(float));
    DHT22.humi = (float * )malloc(n * sizeof(float));
    TSL2560.ligh = (float * )malloc(n * sizeof(float));
    int c; 
    short int TempStatus; 
    for (c = 0; c < n; c++){
        DHT22.temp[c] = generate_random(21.0, 25.0);
        DHT22.humi[c] = generate_random(0.0, 100.0);
        TSL2560.ligh[c] = generate_random(0.1, 40000.0);
    }

    float mean_temp; 
    DHT22.mean = get_mean(DHT22.temp, n);
    DHT22.std_dev = standard_deviation_f(DHT22.temp, DHT22.mean, n);

    for (c = 0; c < n; c+=10)
    {
        float *slice = DHT22.temp + c;  // pointer to the c-th element
        mean_temp = get_mean(slice, 10); // read 10 elements starting from index c
        if (mean_temp < 22.0){ 
            TempStatus = -1;
            printf("\nTOO COLD. Activate resistence, turn down actuators");
            continue; 
        }

        else if (mean_temp > 24.0){
            TempStatus = 1;
            printf("\nTOO HOT. Turn down resistence, activate actuators");
            continue;
        }

        TempStatus = 0; 
        printf("\nALL IDEAL. Turn down resistence, turn down actuators"); 
        
    }  

    printf("\n");
    return 0;
}