#include <stdio.h>

int main() {
    char quote[] = "Think twice";
    char *p = quote;
    printf("%zu\n", sizeof(quote));   // 12 → array still knows its size
    printf("%zu\n", sizeof(p));       // 8  → pointer only knows an address
}

/*
quote knows two things: where it starts, and that it's 12 bytes long (11 chars plus '\0'). p only knows one thing: where it starts. The size is gone.

"A little information has been lost. That loss is called decay."
*/

/*
"Every time you pass an array to a function, you'll decay to a pointer"
*/