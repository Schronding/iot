#include <stdio.h>
#define TRUE 1
#define FALSE 0

int main(){
    int iterations; 
    long int frs_step = 1;
    long int snd_step = 1;
    long int temp_cont;
    printf("\nHow many iterations do you want the Fibonnaci sequence to run? ");
    scanf("%i", &iterations);
    
    for (int index = 1; index <= iterations; index++){
        if (index <= 2) {
            printf("\nIn iteration %i the value is 1\n", index);
        }

        else if (index > 2){
            temp_cont = snd_step;
            snd_step += frs_step;
            frs_step = temp_cont; 
            printf("\nIn iteration %i the value is %i\n", index, snd_step);
        }
    }

    return 0;
}