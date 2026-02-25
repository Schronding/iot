// 1 Calcular media aritmetica
// 2 Calcular desviacion estandar. 
// 3 Estructura sensor
// Arreglo 100 elementos flotantes, media flotante, desviacion estandar flotante. 

// -----------------------------------------------------------------------------
// HOW COMPILATION WORKS WITH LIBRARIES
// -----------------------------------------------------------------------------
// Your own .c files (Utils.c, MeanAndRandom.c) must be listed explicitly in the
// gcc command because the compiler has no idea where they are on your system.
//
// Standard headers like <stdio.h> and <stdlib.h> are part of "libc", which gcc
// links automatically on every build — you never have to ask for it.
//
// <math.h> is different: its compiled implementation lives in a SEPARATE library
// called "libm". It is NOT linked automatically — you must pass the flag: -lm
//
// The pattern is:   -l<name>  →  links against  lib<name>.so / lib<name>.a
//   -lm        → libm       (math:     sqrt, pow, sin, cos, log ...)
//   -lpthread  → libpthread (threads:  pthread_create, pthread_join ...)
//   -lrt       → librt      (realtime: clock_gettime, timers ...)
//   -lz        → libz       (compression: deflate/inflate with zlib)
//   -lcurl     → libcurl    (HTTP/FTP requests)
//   -lssl      → libssl     (encryption, TLS/SSL)
//
// Rule of thumb: if a header is NOT part of the C standard library (libc),
// you will likely need a -l flag to link its implementation.
//
// Full compile command for this file:
//   gcc 25Feb2026.c MeanAndRandom.c Utils.c -o 25Feb2026.bin -lm
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "Utils.h"
#include "MeanAndRandom.h"
#include <math.h>   // Requires -lm at link time (not part of libc)

typedef struct{
    float * mediciones;
    float mean;
    float standard_deviation; 

} sensor; 

int main(){
    int n = 100;
    int c;
    sensor LM35; 
    LM35.mediciones = (float *)malloc(n * sizeof(float));
    for (c = 0; c < n; c++){
        LM35.mediciones[c] = generate_random(24.0, 26.0);
    }

    LM35.mean = get_mean(LM35.mediciones, n);
    float total = 0.0;
    for (c = 0; c < n; c++){
        total += pow((LM35.mediciones[c] - LM35.mean), 2);
    }
    LM35.standard_deviation = sqrt(total / n);
    printf("\nThe mean is: %f", LM35.mean);
    printf("\nThe standard deviation is: %f", LM35.standard_deviation);
    printf("\n");
}