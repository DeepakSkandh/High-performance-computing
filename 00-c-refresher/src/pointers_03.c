#include <stdio.h>

void go_south_east(int *lon , int *lat) {
    *lon = *lon+1;
    *lat = *lat - 1;
}

int main() {
    int latitude = 32;
    int longitude = -42;
    go_south_east(&longitude,&latitude);
    printf("Now at : %i %u\n" ,longitude,latitude);
    return 0;
}