#include <stdio.h>
#define MAX_ARRAY 6

int main(){
    float calificaciones[MAX_ARRAY];
    float total; 
    int index;

    for(int c = 0; c<MAX_ARRAY; c++){
        calificaciones[c] = 0; 
    }
    printf("\nThis program returns the mean of 6 school subjects");
    printf("\nEnter your scores");
    for (index = 0; index < 6; index++){
        printf("\nSubject %i: ", index + 1);
        scanf("%f", &calificaciones[index]);
    }

    for (index = 0; index < 6; index++){
        total += calificaciones[index];
    }
    
    printf("\nYour average is: %.2f \n", total / MAX_ARRAY);

    return 0;
}