#include <stdio.h>
#include <stdlib.h>

int main() {
    char card_name[3];
    puts("Enter the card name: ");
    scanf("%2s",card_name);
    int val = 0;
    if (card_name[0]=='K') {
        val = 10;
    }
    else if (card_name[0] == 'Q') {
        val  = 10;
    }
    else if (card_name[0]== 'J') {
        val = 10;
    }
    else if (card_name[0]=='A') {
        val = 11;
    }
    else {
        printf("Invalid card");
    }
    printf("The value of the card is %i\n",val);
    return 0;
}
/*
That last slot is the key piece. Since a C array doesn't record its own length,
functions like printf("%s"), puts, and strlen need another way to know where the text stops.
The convention is that a string ends at the first byte with value 0, written '\0' and called the null terminator.
These functions start at the first character and keep reading until they reach it.*/