/* Resuelva los siguientes problemas utilizando el lenguaje C.

1. Diseña un programa que lea un número flotante por teclado y muestre por 
pantalla el mensaje <<El número es negativo.>>  sólo si el numero es negativo.

2. Diseña un programa que lea la edad de dos personas y diga quién es más joven, 
la primera o la segunda. Ten en cuenta que ambas pueden tener la misma edad. 
En tal caso, hazlo saber con un mensaje adecuado.

3. Diseña un programa que, dado un número entero, muestre por pantalla el mensaje 
<<El número es par.>> cuando el numero sea par y el mensaje 
<<El número es impar.>> cuando sea impar.

(Una pista: un número es par si el resto de dividirlo por 2 es 0, e impar en caso 
contrario.)

4. Diseña un programa que, dado un número entero, determine si éste es el doble 
de un numero impar. */

#define clearbuffer() do { int c; while ((c = getchar()) != '\n' && c != EOF) { } } while (0)
#include <stdio.h>

void negativeNumber(){
    int number;
    printf("\nEnter your number: ");
    scanf("%i", &number);
    if (number < 0) {
        printf("\nYour number is negative");
    }
    else{
        printf("\nYour number is positive");
    }
}

void twoAges(){
    int age1;
    int age2;
    printf("\nEnter the first person's age: ");
    scanf("%i", &age1);
    printf("\nEnter the second person's age: ");
    scanf("%i", &age2);
    if (age1 == age2){
        printf("Both people share the same age"); 
    }

    if (age1 > age2){
        printf("\nThe first person is older than the second one");
    }
    else{
        printf("\nThe second person is older than the first one");
    }
}

void ParOrImpar(){
    int number; 
    printf("\nEnter the number: ");
    scanf("%i", &number);
    if ((number % 2) == 0){
        printf("\nYour number is par");
    }
    else{
        printf("\nYour number is impar");
    }
}

void isDouble(){
    int number; 
    printf("\nEnter the number: ");
    scanf("%i", &number);
    if ((number / 2) % 2 == 1){
        printf("Your number is the double of an impar one");
    }
    else{
        printf("Your number is NOT the double of an impar one"); 
    }
}

int main(){
    negativeNumber();
    twoAges();
    ParOrImpar();
    isDouble();

    return 0; 
}