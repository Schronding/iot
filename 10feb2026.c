#include <stdio.h>

/* 
Constantes */
#define TRUE 1
#define FALSE 0
#define clearbuffer() do { int c; while ((c = getchar()) != '\n' && c != EOF) { } } while (0)

/*
Variables globales
Prototipo de funciones
*/

int main(){
    char opt;
    float side;
    float side2;
    float radius;
    short int flag = TRUE; 

    do {
    printf("Ingresa una opcion: ");
    printf("\n1. Area cuadrado");
    printf("\n2. Area circulo");
    printf("\n3. Area rectangulo");
    printf("\n4. Area triangulo equilatero");
    printf("\n5. Area hexagono");
    printf("\n6. Salir");
    printf("\nQue deseas hacer?: \n");
    scanf("%c", &opt);
    
    
        switch (opt){
            case '1':
                printf("\nArea Cuadrado \n");
                printf("Ingresa el lado: ");
                scanf("%f", &side);
                printf("\nArea es: %.4f", side * side);
                printf("\n");
                break;
            
            case '2':
                printf("\nArea Circulo \n");
                printf("Ingresa el radio: ");
                scanf("%f", &radius);
                printf("\nArea es: %.4f", 3.1415 * (radius * radius));
                printf("\n");
                break;
            
            case '3':
                printf("\nArea Rectangulo \n");
                printf("Ingresa el ancho: ");
                scanf("%f", &side2);
                printf("Ingresa la altura: ");
                scanf("%f", &side);
                printf("\nArea es: %.4f", side * side2);
                printf("\n");
                break;
            
            case '4':
                printf("\nArea Triangulo Equilatero \n");
                printf("Ingresa el ancho: ");
                scanf("%f", &side);
                printf("Ingresa la altura: ");
                scanf("%f", &side2);
                printf("\nArea es: %.4f", side * side2 / 2);
                printf("\n");
                break;
            
            case '5':
                printf("\nArea Hexagono \n");
                printf("Ingresa el perimetro: ");
                scanf("%f", &side);
                printf("Ingresa el apotema: ");
                scanf("%f", &radius);
                printf("\nArea es: %.4f", side * radius / 2);
                printf("\n");
                break;
            
            case '6':
                printf("Nos vemos pa");
                flag = FALSE; 
                printf("\n");
                break;
            
            default:
                printf("\nOpcion no valida :c");
            }
            
            clearbuffer();

        } while (flag == TRUE);

    return 0;
}