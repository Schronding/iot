// Dados a, b, c muestre en pantalla
// Mayor de las 3
// Menor de las 3
// Si son iguales

#include <stdio.h>

int main(){
    int a;
    int b;
    int c;

    printf("Ingresa tu primer numero: ");
    scanf("%i", &a);

    printf("Ingresa tu segundo numero: ");
    scanf("%i", &b);

    printf("Ingresa tu tercer numero: ");
    scanf("%i", &c);

    printf("\nTu primer numero fue %i", a);
    printf("\nTu segundo numero fue %i", b);
    printf("\nTu tercer numero fue %i", c);

    int mayor;
    int medio;
    int menor;

    if (a >= b && a >= c) {
        mayor = a;
        if (b >= c) {
            medio = b;
            menor = c;
        } else {
            medio = c;
            menor = b;
        }
    } else if (b >= a && b >= c) {
        mayor = b;
        if (a >= c) {
            medio = a;
            menor = c;
        } else {
            medio = c;
            menor = a;
        }
    } else {
        mayor = c;
        if (a >= b) {
            medio = a;
            menor = b;
        } else {
            medio = b;
            menor = a;
        }
    }

    printf("\nEl mayor de los tres numeros es %i", mayor);
    printf("\nEl segundo mayor de los tres numeros es %i", medio);
    printf("\nEl menor de los tres numeros es %i", menor);

    if (a == b && b == c) {
        printf("\nLos tres numeros son iguales con valor %i", a);
    } else if (a == b || a == c || b == c) {
        int valor_igual = (a == b || a == c) ? a : b;
        printf("\nDos numeros son iguales con valor %i\n", valor_igual);
    }
}

