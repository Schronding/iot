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

char menu(){
    char opt;
    printf("Ingresa una opcion: ");
    printf("\n1. Area cuadrado");
    printf("\n2. Area circulo");
    printf("\n3. Area rectangulo");
    printf("\n4. Area triangulo equilatero");
    printf("\n5. Area hexagono");
    printf("\n6. Salir");
    printf("\nQue deseas hacer?: \n");
    scanf("%c", &opt);
}

void areaSquare(){
    float side;
    printf("\nArea Cuadrado \n");
    printf("Ingresa el lado: ");
    scanf("%f", &side);
    printf("\nArea es: %.4f", side * side);
    printf("\n");
}

void areaCircle(){
    float radius;
    printf("\nArea Circulo \n");
    printf("Ingresa el radio: ");
    scanf("%f", &radius);
    printf("\nArea es: %.4f", 3.1415 * (radius * radius));
    printf("\n");
}

void areaRectangle(){
    float side;
    float side2;
    printf("\nArea Rectangulo \n");
    printf("Ingresa el ancho: ");
    scanf("%f", &side2);
    printf("Ingresa la altura: ");
    scanf("%f", &side);
    printf("\nArea es: %.4f", side * side2);
    printf("\n");
}

void areaEqTriangle(){
    float width;
    float height; 
    printf("\nArea Triangulo Equilatero \n");
    printf("Ingresa el ancho: ");
    scanf("%f", &width);
    printf("Ingresa la altura: ");
    scanf("%f", &height);
    printf("\nArea es: %.4f", width * height / 2);
    printf("\n");
}

void areaHexagon(){
    float perimeter;
    float apotema;
    printf("\nArea Hexagono \n");
    printf("Ingresa el perimetro: ");
    scanf("%f", &perimeter);
    printf("Ingresa el apotema: ");
    scanf("%f", &apotema);
    printf("\nArea es: %.4f", perimeter * apotema / 2);
    printf("\n");
}

int factorial(){
    int iterations;
    printf("\nHow many iterations do you want?\t");
    scanf("%i", &iterations);
    if (iterations < 0) {
        printf("\nNot a valid option.\nFactorials are defined only for positive integers\n");
        return 0;
    }
    unsigned long long int total = 1; 
    while (iterations > 1){
        total *= iterations;
        iterations--;
    }
    printf("\nYour total is %lli\n", total);
}

int main(){
    factorial();
    return 0;

    short int flag = TRUE; 

    do {
        switch (menu()){
            case '1':
                areaSquare();
                break;
            
            case '2':
                areaCircle();
                break;
            
            case '3':
                areaRectangle();
                break;
            
            case '4':
                areaEqTriangle();
                break;
            
            case '5':
                areaHexagon();
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