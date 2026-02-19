#include <stdio.h>
#define MAXARRAY 4
#include "Utils.h"



int main(){
    float arreglo_pettite [MAXARRAY] = {80.0, 50.0, 83.2, 51.2};

    float valor = 80.0; 

    printf("\nElemento %f encontrado en la posicion %d\n", valor, linear_search(arreglo_pettite, valor, MAXARRAY));
    printf("\n --- Desorganized array --- \n");
    PrintArrayFlo(arreglo_pettite, MAXARRAY);
    printf("\n --- Organized array --- \n");
    PrintArrayFlo(BubbleSort(arreglo_pettite, MAXARRAY), MAXARRAY);
    printf("\nMinimum Value: %f", MinMax(arreglo_pettite, MAXARRAY, "min"));
    printf("\nMaximum Value: %f", MinMax(arreglo_pettite, MAXARRAY, "max"));
    printf("\n");
}

