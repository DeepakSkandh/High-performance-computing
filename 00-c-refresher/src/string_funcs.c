#include <stdio.h>
#include <string.h>
int main() {
    char tracks[][80] = {
        "I left my heart in Harvard Med School",
        "Newark, Newark - a wonderful town",
        "Dancing with a Dork",
        "From here to maternity",
        "The girl from Iwo Jima",
    }; 
    strcat(tracks[0], tracks[1]); // concatenate string
    int x = strlen(tracks[0]); // len of string
    printf("%s\n",tracks[0]);
    printf("%i\n",x);
    return 0;
}



/*
string.h can comapare two strings,search for a string,make a copy of a string and slice the string.
*/