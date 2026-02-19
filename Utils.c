#include <stdio.h>
#include <string.h>

int linear_search(float arr[], float value, int max_array){
    int c; 

    for (c = 0; c < max_array; c++){
        if(arr[c] == value)
            return c;
    }

    return -1;

}

float * BubbleSort(float arr[], int max_array){
    int i, j, temp;

    for (i = 0; i < max_array - 1; i++){
        for (j = 0; j < max_array - i - 1; j++){
            if (arr[j] > arr[j + 1]){
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    return arr;
}

/* When you create a function that works with arrays, you always need to provide as a parameter the size of that array. */

void PrintArrayFlo(float arr[], int maxarray){
    int c;
    for (c = 0; c < maxarray; c++){
        printf("\nValue %f in %i index", arr[c], c);
    }

    printf("\n");
}

void PrintArrayInt(int arr[], int maxarray){
    int c;
    for (c = 0; c < maxarray; c++){
        printf("\nValue %f in %i index", arr[c], c);
    }

    printf("\n");
}

float MinMax(float arr[], int max_array, char * type){
    float * temp_array = BubbleSort(arr, max_array);
    if (!strcmp(type, "min"))
        return temp_array[0];
    else if (!strcmp(type, "max"))
        return temp_array[max_array - 1];
    else
        printf("\n --- ERROR: Not a valid option --- \n");
}