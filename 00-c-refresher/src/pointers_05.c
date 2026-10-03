#include <stdio.h>

int main() {
    char quote[] = "Think twice";
    char *p = quote;
    printf("%p\n",(void *)quote);
    printf("%p\n",(void *)&quote); // address of the whole array → same!
    printf("%p\n",(void *)p);
    return 0;
}