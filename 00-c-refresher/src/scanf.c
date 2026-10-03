#include <stdio.h>

int main() {
    char first_name[10];
    char last_name[10];
    printf("Enter first name then last name\n");
    scanf("%9s %9s",first_name,last_name);
    printf("first : %s\n last : %s\n",first_name,last_name);
    return 0;
}