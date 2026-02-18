#include <stdio.h>
#include <stdlib.h>
#include "MeanAndRandom.h"

typedef struct{
    float presion;
    int humedad;
    float temp;
    int luminosidad;
    float vel_viento;
    int CO2; 
    float acc[3];
} type_mediciones;

/* Arrays are exclusively homogeneous in C, but you might make use pointers, which is flexible but risky*/

/*
struct mediciones{
        float presion;
        int humedad;
        float temp;
        int luminosidad;
        float vel_viento;
        float CO2; 
        float acc[3];
    };
*/

int main(){
    /* When you use typedef you avoid having to put struct later down the code. Note how the line below
    requires that the struct keyword is used. */
    /* estacion_1.presion = 807.0;  hPa 
    estacion_1.temp = 31.6;
    estacion_1.acc[0] = 0; 
    estacion_1.acc[1] = 0; 
    estacion_1.acc[2] = 9.81; /* m / s^2 
    printf("\nPresion [hPa]: %f\n", estacion_1.presion);
    printf("\nAceleracion [g]: %f\n", estacion_1.acc[2]); */

    /* The struct is 5 floats and 2 integers*/
    int n = 3;
    int c;
    type_mediciones * estaciones = (type_mediciones *)malloc(n * sizeof(type_mediciones)); 


    for (c = 0; c < n; c++){
        estaciones[c].presion = generate_random(800.0, 900.0);
        estaciones[c].humedad = (int)generate_random(0.0, 100.0);
        estaciones[c].temp = generate_random(0.0, 273.0);
        estaciones[c].luminosidad = (int)generate_random(0.0, 100.0);
        estaciones[c].vel_viento = generate_random(0.0, 5000.0);
        estaciones[c].CO2 = (int)generate_random(400.0, 10000.0);
        estaciones[c].acc[0] = 0.0;
        estaciones[c].acc[1] = 0.0;
        estaciones[c].acc[2] = generate_random(0, 9.81);
    }

    float * presiones = (float *)malloc(n * sizeof(float)); 
    float * humedades = (float *)malloc(n * sizeof(float)); 
    float * temperaturas = (float *)malloc(n * sizeof(float)); 
    float * luminosidades = (float *)malloc(n * sizeof(float)); 
    float * velocidades_viento = (float *)malloc(n * sizeof(float)); 
    float * arr_CO2 = (float *)malloc(n * sizeof(float)); 
    float * aceleraciones_x = (float *)malloc(n * sizeof(float)); 
    float * aceleraciones_y = (float *)malloc(n * sizeof(float)); 
    float * aceleraciones_z = (float *)malloc(n * sizeof(float)); 

    for (c = 0; c < n; c++){
        presiones[c] = estaciones[c].presion; 
        humedades[c] = estaciones[c].humedad; 
        temperaturas[c] = estaciones[c].temp; 
        luminosidades[c] = estaciones[c].luminosidad; 
        velocidades_viento[c] = estaciones[c].vel_viento; 
        arr_CO2[c] = estaciones[c].CO2; 
        aceleraciones_x[c] = estaciones[c].acc[0]; 
        aceleraciones_y[c] = estaciones[c].acc[1]; 
        aceleraciones_z[c] = estaciones[c].acc[2]; 
    }

    printf("\nThe mean of the pressure of all stations is %f", get_mean(presiones, n));
    printf("\nThe mean of the humidity of all stations is %i", (int)get_mean(humedades, n));
    printf("\nThe mean of the temperature of all stations is %f", get_mean(temperaturas, n));
    printf("\nThe mean of the wind velocity of all stations is %i", (int)get_mean(luminosidades, n));
    printf("\nThe mean of the CO2 of all stations is %f", get_mean(velocidades_viento, n));
    printf("\nThe mean of the downward acceleration of all stations is %i", (int)get_mean(arr_CO2, n));
    printf("\nThe mean of the downward acceleration of all stations is %f", get_mean(aceleraciones_x, n));
    printf("\nThe mean of the downward acceleration of all stations is %f", get_mean(aceleraciones_y, n));
    printf("\nThe mean of the downward acceleration of all stations is %i\n", (int)get_mean(aceleraciones_z, n));

    return 0;
}



/* float get_mean(float * arr, int n); */
/* float generate_random(float min, float max); */

/* I can define structures outside the 'main' function. */