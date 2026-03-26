#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "Statistics.h"
#include <math.h>



/* It seems that the module operator doesn't work with floats*/
float generate_random(float min, float max){
    float temperature;     
    temperature = min + ((float)rand() / (float)RAND_MAX) * (max - min);

    return temperature; 
}

/* Attempt to see if this appears in VSCode when hovering on the function*/
float standard_deviation_f(float * array, float mean, int max_array){
    float total = 0;
    for (int c = 0; c < max_array; c++){
        total += pow((array[c] - mean), 2);
    }
    return sqrt(total / max_array);
}

float get_mean(float * arr, int n){
    int c;
    float total = 0;
    for (c = 0; c < n; c++) total += arr[c];

    return (total / n);
}

