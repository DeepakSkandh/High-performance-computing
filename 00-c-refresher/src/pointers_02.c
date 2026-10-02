#include <stdio.h>

int main() {
    int val = 42;
    int *p = &val;
    int new = *p;
    *p = 84;
    printf("%p\n",(void *)p);
    printf("%p\n",(void *)&val);
    printf("%u\n",*p); //dereferencing
    printf("%u\n",new);
    printf("%p\n",(void *)p);
    printf("%p\n",(void *)&new);
    return 0;
}