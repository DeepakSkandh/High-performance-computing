/*
If you declare a 
variable inside a function like main(), the computer 
will store it in a section of memory called the stack. If 
a variable is declared outside any function, it will be stored 
in the globals section of memory
*/

/*
The stack is small. It's typically about 8 MB on Linux (ulimit -s shows it). 
In HPC code, a big local array like double A[2000][2000]; (about 32 MB) will crash with a stack overflow. 
Make it global, static, or allocate it with malloc.
*/

# include <stdio.h>

int main() {
    int val = 42;
    int *p = &val; // *--> is a declaration that p is a pointer to int , & --> adress of val
    printf("%u\n",val);
    printf("%p\n",(void *)p); // address
    printf("%p\n",(void *)&val); // address
    return 0;
}