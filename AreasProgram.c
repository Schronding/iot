#include <stdio.h>
#include <math.h>

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
    printf("\nWhat is the number you want to calculate its factorial?\t");
    scanf("%i", &iterations);
    if (iterations < 0) {
        printf("\nNot a valid option.\nFactorials are defined only for positive integers\n");
        return 0;
    }

    unsigned long long fac_arr[iterations];
    for (int i = 0; i < iterations; i++){
        fac_arr[i] = 0;
    }


    unsigned long long int total = 1; 

    while (iterations > 1){
        total *= iterations;
        iterations--;
        // fac_arr[iterations] = total;
    }

    printf("\nYour total is %lli\n", total);

    /*for (int i = 0; i < iterations; i++){
        printf("\nValue in index %i is %lli\n", i, fac_arr[i]);
    }*/
}

#define POSITIVE 1 
#define NEGATIVE 0 

void sine(){
    double x;
    double calc_x; 
    int appr_terms = 6;
    printf("\nThis program calculates sin(x).");
    printf("\nIntroduce your desired value of x:\t");
    scanf("%d", x);
    for (int i = 1; i <= 13; i += 2){
        calc_x += (long long int)(pow(calc_x, i) / factorial(i));
        if (i % 2 == 1){
            calc_x *= 1; 
        }
        else{
            calc_x *= -1; 
        }
    }

    double math_sin = sin(x);
    printf("\n Library: %d | Approximation: %d | Error: %d", math_sin, calc_x, math_sin - calc_x);
}

int main(){
    sine();
    return 0;
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