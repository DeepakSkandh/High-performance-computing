#include <stdio.h>

int main() {
    printf("%d\n",-42); // signed int
    printf("%i\n",-52); // signed int
    printf("%u\n",100); // unsigned int
    printf("%f\n",100.0); // float
    printf("%a\n",1.0); // hex float
    printf("%c\n",'A'); // single character
    printf("100%%\n"); //literal
    printf("%i %i\n",10,100); 
    return 0;
}