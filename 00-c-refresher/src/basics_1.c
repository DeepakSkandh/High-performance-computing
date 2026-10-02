#include<stdio.h>

int main() {
    int x = 65;
    printf("%d\n",x);
    printf("%c\n",x);
    printf("%x\n",x);
    printf("%o\n",x);
    return 0;

}

/*
printf is a variadic function.No type of infromation goes inside printf , inside printf bytes are just bits .
The format string is the only source of that information.
 Each % specifier is effectively an instruction: "fetch the next argument, treat it as this type, and render it this way."
*/