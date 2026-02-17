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

int factorial(int iterations){
    // int iterations;
    // printf("\nWhat is the number you want to calculate its factorial?\t");
    // scanf("%i", &iterations);
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

    /* I was missing the return statement! Without it the function prints but never gives
    the value back to whoever called it, so the division in sine() was using garbage. */
    return total;

    /*for (int i = 0; i < iterations; i++){
        printf("\nValue in index %i is %lli\n", i, fac_arr[i]);
    }*/
}

#define POSITIVE 1 
#define NEGATIVE 0 

/* As radians are a unit for measuring the angles based on the radius of a circle I can't simply pass degrees to <math.h> 
sin(x) function. One radian is the angle formed when the arc length equals the radius. In order to make the program work I first
need to do the conversion from degrees to radians. */

    /* I was using %ld which is for long int. For long double the correct specifier is %Lf. 
    That's why x always had garbage — scanf was writing the bytes in the wrong format. */

    /* The Taylor series for sin(x) is: x - x^3/3! + x^5/5! - x^7/7! + ...
    Each term can be computed from the previous one without calling pow() and factorial()
    every time. The recurrence is: term_next = term_prev * (-x*x) / (i*(i-1))
    This avoids overflow in factorial and keeps everything in floating point. 
    
    My original mistakes were:
    1. I was using calc_x (the accumulator) inside pow() instead of the original x,
       so each iteration fed a corrupted value into the next.
    2. I was casting to (long long int) which destroyed the decimal precision.
    3. The sign logic (multiply by 1 or -1 after adding) was wrong — the sign
       needs to be part of each term BEFORE adding it, not applied to the whole sum.
    4. factorial() had no return statement so division used garbage. */

        /* I was using %ld and %lli for doubles and long doubles — those are for integers.
    For double use %f, for long double use %Lf. That's why everything printed as 0. */

void sine(){
    long double x;
    long double calc_x; 
    printf("\nThis program calculates sin(x).");
    printf("\nIntroduce your desired value of x in radians:\t");
    scanf("%Lf", &x);


    calc_x = x;              /* first term of the series is just x */
    long double term = x;    /* track each term separately */
    for (int i = 3; i <= 13; i += 2){
        term *= -x * x / (i * (i - 1));   /* derive next term from previous */
        calc_x += term;
        printf("Term i=%i \t| \t Partial sum: %Lf\n", i, calc_x);
    }

    double math_sin = sin((double)x);
    printf("\n Library: %f | Approximation: %Lf | Error: %Lf \n",
        math_sin, calc_x, (long double)math_sin - calc_x);
}

/* There are so many things about this program that I don't understand. When I changed 'calc_x' for long int the approximation
of the library turn to 0, but how is that even possible? I calculate math_sin directly with 'x'. I thought that maybe my mistake
was that I was not initializing the value of 'calc_x', that is why it was always on zero, but even now that doesn't seem to work.
Even if the library is incorrect why am I not getting any value in error? And if all my values are 0 on my terminal, why at the
end the approximation is equal to one? */

/* I am getting a segmentation fault, which seems to be common. It seems that it is a runtime error that occurs when a program
tries to access memory it's not allowed to access. Common causes are
1. Dereferencing NULL or unitialized pointers.
2. Accessing freed memory
3. Buffer overflow (writing beyond array bounds)
4. Stack overflow (infinite recursion or large local arrays)
5. Writing to read-only memory

It seems I can use gdb, but I don't have it installed as it appears that the command is not found.

I can install it with `sudo apt-get install gdb`

The -g flag in gcc tells the program to include debugging symbols, as:
- Variable names
- Function names
- Line numbers from the source code. 

GDB stands for GNU Debugger and it is a tool for debugging that can allow me to
- Run the program step-by-step
- Inspect variable values at any point
- See exactly where the program crashes
- Set breakpoints to pause execution
- Examine the call stack

*/

int main(){
    sine();
    return 0;
    //factorial();
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