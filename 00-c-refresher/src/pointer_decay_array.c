/*
"Every time you pass an array to a function, you'll decay to a pointer"
*/
#include <stdio.h>
void show(char msg[]) {               // looks like an array...
    printf("%zu\n", sizeof(msg));     // ...but prints 8: it's really char *msg
}

int main() {
    char quote[] = "Think twice";
    printf("%zu\n", sizeof(quote));   // 12
    show(quote);                      // 8
    return 0;
}