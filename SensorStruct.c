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

// -----------------------------------------------------------------------------
// STRUCT ACCESS: "." vs "->"
// -----------------------------------------------------------------------------
// Both operators access a struct member. The ONLY difference is what's on
// the left side:
//
//   lm35.mean      →  lm35 is a struct (direct value on the stack)
//   lm35->mean     →  lm35 is a POINTER to a struct (data lives on the heap)
//
// "->" is just syntactic sugar:  lm35->mean  ==  (*lm35).mean
// The compiler generates identical machine code — zero performance difference.
//
// THE REAL TRADEOFF IS STACK vs HEAP, not "." vs "->":
//
//   Stack  (uses ".")
//     + Fast: data is nearby in memory, very cache-friendly
//     + Automatic cleanup when the function returns
//     - Limited size (~1–8 MB total)
//     - Copying a large struct to another function is expensive (full copy)
//
//   Heap   (uses "->", requires malloc/free)
//     + Virtually unlimited size
//     + Passing a pointer to a function is always cheap (just 8 bytes)
//     + Data can outlive the function that created it
//     - Slightly slower if data is not in CPU cache (pointer dereference)
//     - YOU must free() it manually — forgetting causes memory leaks
//
// Rule of thumb:
//   Small, short-lived struct  →  stack + "."
//   Large struct, dynamic data, or data shared across functions  →  heap + "->"
//
// Example from this file:
//   LM35.mean           →  LM35 is a sensor struct on the stack
//   LM35.mediciones     →  LM35 is on the stack, but mediciones is a float*
//                          pointing to heap memory allocated with malloc()
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "Utils.h"
#include "Statistics.h"
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
    LM35.standard_deviation = standard_deviation_f(LM35.mediciones, LM35.mean, n); 
    printf("\nThe mean is: %f", LM35.mean);
    printf("\nThe standard deviation is: %f", LM35.standard_deviation);
    printf("\n");
}