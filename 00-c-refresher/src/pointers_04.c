#include <stdio.h>

int main() {
    char quote[] = "India plays cricket";
    printf("%s\n",quote);// C turns quote into the address of 'I', then dereferences → I
    printf("%zu\n", sizeof(quote));   // 20 → size of the whole array
    return 0;
}

/*
A real pointer would give 8 (the size of one address). quote gives 21 because it's the whole array.

So the summary: quote is an array, but it behaves like a pointer to its first character when you use it.
*/