#include <stdio.h>

// Sea f(x) = x^2 + x + 3
// x pertenece a [-3, 3]
// Delta = 1 * 10^-2

int main(){
    float x; 
    float fx;
    x = -3.0;
    while(x <= 3){
        fx = (x * x) + x + 3;
        printf("\n%f\t|\t%f", x, fx);
        x += 0.01;  
    }

    return 0;
}