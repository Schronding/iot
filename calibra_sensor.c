#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MEAS 100

struct sensor{
    float mediciones[MAX_MEAS];
    float t_mean;
    float t_devstd;
};

float generate_random_float(float min, float max) {
    float scale = (float)rand() / (float)RAND_MAX; /* [0, 1.0] */

    return min + scale * (max - min);
}

void fill_temperatures(struct sensor *sen){
    for(int c=0; c<MAX_MEAS; c++)
        sen->mediciones[c] = generate_random_float(24.0, 26.0);
}

void print_temperatures(struct sensor *sen){
    for(int c=0; c<MAX_MEAS; c++)
        printf("%f\n", sen->mediciones[c]);
}

void get_mean(struct sensor *sen){
    float mean = 0;

    for(int c=0; c<MAX_MEAS; c++)
        mean += sen->mediciones[c];

    sen->t_mean = mean/MAX_MEAS;
}

void get_std_dev(struct sensor *sensor){
    float total = 0;
    int c;
    for (int c = 0; c < MAX_MEAS; c++){
        total += pow((sensor->mediciones[c] - sensor->t_mean), 2);
    }
    sensor->t_devstd = sqrt(total / MAX_MEAS);
}


int main(){
    srand((unsigned int)time(NULL));

    struct sensor lm35;

    fill_temperatures(&lm35);
    print_temperatures(&lm35);

    return 0;
}
