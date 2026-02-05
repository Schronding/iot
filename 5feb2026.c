// Dados a, b, c muestre en pantalla
// Mayor de las 3
// Menor de las 3
// Si son iguales

#include <stdio.h>
#include <ar.h>

int main(){
    int a;
    int b;
    int c;

    printf("Ingresa tu primer numero: ");
    scanf("%i", a);

    printf("Ingresa tu segundo numero: ");
    scanf("%i", b);

    printf("Ingresa tu tercer numero: ");
    scanf("%i", c);

    printf("Tu primer numero fue %i", a);
    printf("Tu segundo numero fue %i", b);
    printf("Tu tercer numero fue %i", c);


    int local_max; 
    int orden_array[] = {a, b, c}; 

    void bubble_sort(arr){
        for (int i = 0; lenght(arr) - 1; i++){
            if arr[i] > arr[i + 1]{
                int loc_max = arr[i]
                arr[i] = arr[i + 1]
                arr[i + 1] = loc_max
            }
        }

        for (int i = 0; lenght(orden_array) - 1; i++){
        if (arr[i] == arr[i + 1]) && ((i+1) < length(orden_array)){
            if (arr[i + 1] == arr[i + 2]) && ((i+2) < length(orden_array)){
                printf("Los tres numeros son iguales con valor %i", arr[i])
                break
            }
            else{
                printf("Los dos numeros son iguales con valor %i", arr[i])
                printf("El valor minimo es %i", arr[i + 2])
            }
        }
        }
    }

    bubble_sort(arr);
}

