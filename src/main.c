#include <stdio.h>

#include "../include/bigint.h"

int main(int argc, char const *argv[]) {
    BigInt* bigint = BigInt_FromString("999999999999999999999999999999999999");
    BigInt* bigint2 = BigInt_FromString("999999999999999999999999999999999999");
    BigInt* sum = BigInt_Init(0);
    BigInt_Add(bigint, bigint2, sum);

    char* str = BigInt_ToString(sum);
    int res = strcmp(str, "1999999999999999999999999999999999998");
    printf("str: %s\n", str);
    printf("res: %d\n", res);

    free(str);
    BigInt_Free(bigint);
    BigInt_Free(bigint2);
    BigInt_Free(sum);
    // BigInt* bigint = BigInt_FromString("12863437456928734652983746517826345178263417263451872634517623");
    // BigInt* bigint2 = BigInt_FromString("12863437456928734652983746517826345178263417263451872634517623");
    // BigInt* sum = BigInt_Init(0);
    // BigInt_Add(bigint, bigint2, sum);

    // char* str = BigInt_ToString(sum);
    // int res = strcmp(str, "25726874913857469305967493035652690356526834526903745269035246");
    // printf("str: %s\n", str);
    // printf("res: %d\n", res);

    // free(str);
    // BigInt_Free(bigint);
    // BigInt_Free(bigint2);
    return 0;
}
