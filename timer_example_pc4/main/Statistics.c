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
    int c;
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

/*
int main(){
    srand(time(NULL));
    printf("\nHow many registers are you going to insert?\t");
    int n;
    scanf("%i", &n);
    float * temp = (float *)malloc(n * sizeof(float)); 

    if (temp != NULL ){ /* In order to know if we have enough memory to store the temp pointer. 
        int c; 
        for (c = 0; c < n; c++){
            temp[c] = generate_random(12, 28); 
        }

        for (c = 0; c < n; c++){
            printf("\nTemperature %i is: %f\n", c + 1, temp[c]);
        }

        printf("\nMean: %f\n", get_mean(temp, n));

        free(temp);
    }

    return 0; 
}
*/

/* The difference between malloc and alloc is that the former needs one argument while the latter needs two. 

To use malloc you need <stdlib.h> 


*/