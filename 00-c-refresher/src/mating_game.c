#include <stdio.h>

int main() {
    int contestants[] = {1,2,3};
    int *choice = contestants;
    contestants[0] = 2;
    contestants[1] = contestants[2];
    contestants[2] = *choice;
    printf("I am going to pick %i\n",contestants[2]);
    printf("Array starts at %p\n", (void *)contestants);
    printf("Array is ");
    
    for (int i =0;i<3;i++) {
        printf("%u ",contestants[i]);
    }
    return 0;
}