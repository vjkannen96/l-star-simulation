#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void lstar(int* array, int length){
    for(int i = 0; i < length; i++){
        printf("%d", array[i]);
    }
    printf("\n");
}

void tryAllSets(int* array, int index, int length){
    if(index == length - 1){
        array[index] = 0;
        lstar(array, length);
        array[index] = 1;
        lstar(array, length);
        return;
    }
    else{
        array[index] = 0;
        tryAllSets(array, index + 1, length);
        array[index] = 1;
        tryAllSets(array, index+1, length);
    }
}

int main(void){

    int teacher[5] = {};
    tryAllSets(teacher, 0, 5);
    return 0;
}