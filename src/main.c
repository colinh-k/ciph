#include <stdio.h>

#include "../include/bigint.h"

int main(int argc, char const *argv[]) {
    BigInt* bigint = BigInt_FromString("12863437456928734652983746517826345178263417263451872634517623");
    for (size_t i = bigint->nDigits - 1; i != (size_t) -1; i--) {
        printf("%hhu", bigint->digits[i]);
    }
    printf("\n");
    return 0;
}
