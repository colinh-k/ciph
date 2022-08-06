#ifndef BIGINT_H_
#define BIGINT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// require that the result of a digit_t * digit_t <= BigInt
// which means the result of digit_t * digit_t can be stored
// in a BigInt.
// NOTE: implicitly, we have chosen the base B = 2^16
// for the numerical rep
// typedef uint16_t digit_t;
// a type large enough to store the result of an operation
// performed on two digit_t's
// typedef uint32_t result_t;

// base is 2^16
// const uint64_t BASE = 1 << (sizeof(digit_t) * 8);

typedef uint8_t digit_t;


#define BI_MSDPTR(x) &((x)->digits[(x)->nDigits - 1])
#define BI_LSDPTR(x) (x)->digits

// rep notes: evalutation of digits increases with increasing
// index into the 'digits' array in this struct.
// when digits == NULL, it is considered ZERO (0)
typedef struct {
    bool isNegative;  // true if bigint is negative
    size_t nDigits;
    size_t capacity;
    digit_t* digits;
} BigInt;

typedef enum {
    E_SUCCESS,
    E_FAILURE,
    E_OVERFLOW,
    E_UNDERFLOW,
    E_DIVISION_BY_ZERO,  // divizion by zero
    E_MEMORY_ALLOCATION,  // memory allocation error
    E_MODULUS,  // Montgomery multiplication given even modulus
    E_NULL_ARGUMENT  // NULL passed as argument
} BigInt_Error_t;

typedef struct {
    // TODO: is it necessary to store the code in this struct ?
    // we can just map from the error code to the index in the
    // string arrary
    BigInt_Error_t eCode;
    const char* msg;
} BigInt_ErrorDesc_t;

BigInt_ErrorDesc_t eDesc[] = {
    { E_SUCCESS, "operation successful" },
    { E_FAILURE, "operation failure" },
    { E_OVERFLOW, "operation resulted in overflow" },
    { E_UNDERFLOW, "operation resulted in underflow" },
    { E_DIVISION_BY_ZERO, "operation attempted division by zero" },
    { E_MEMORY_ALLOCATION, "memory allocation failed" },
    { E_MODULUS, "Montgomery multiplication was given an even modulus" },
    { E_NULL_ARGUMENT, "operation was given a NULL pointer argument" }
};

void BigInt_PrintError(BigInt_Error_t code) {
    fprintf(stderr, "%s\n", eDesc[code].msg);
}

BigInt* BigInt_Init(size_t capacity);
BigInt* BigInt_FromString(const char* str);
// BigInt* BigInt_Copy(BigInt* other);
// int8_t  BigInt_Compare(BigInt* other);
void BigInt_Free(BigInt* bigInt);
BigInt_Error_t BigInt_Add(BigInt* left, BigInt* right, BigInt* result);

BigInt* BigInt_Init(size_t capacity) {
    BigInt *ret = (BigInt*) malloc(sizeof(BigInt));
    if (!ret)
        return NULL;
    ret->isNegative = false;
    ret->capacity = capacity;
    ret->nDigits = 0;
    ret->digits = (digit_t*) malloc(sizeof(digit_t) * capacity);
    if (!(ret->digits))
        return NULL;
    return ret;
}

BigInt* BigInt_FromString(const char* str) {
    bool isNegative = false;
    if (str[0] == '-') {
        isNegative = true;
        str++;  // ignore the leading dash
    }

    size_t nDigits = strlen(str);
    BigInt *ret = BigInt_Init(nDigits);
    if (!ret)
        return NULL;

    ret->nDigits = nDigits;
    ret->isNegative = isNegative;
    for (size_t i = nDigits - 1; i != (size_t) -1; i--) {
        char c = str[i];
        if (isdigit(c)) {
            ret->digits[nDigits - 1 - i] = (digit_t) (c - '0');
        } else {
            // invalid character for integer type (i.e., NaN)
            return NULL;
        }
    }
    return ret;
}

void BigInt_Free(BigInt* bigint) {
    free(bigint->digits);
    free(bigint);
}

BigInt_Error_t BigInt_Add(BigInt* left, BigInt* right, BigInt* result) {
    if (left == NULL || right == NULL || result == NULL)
        return E_NULL_ARGUMENT;

    // init ptrs so a points to longer int or a, and b
    // points to shorter int or b
    // BigInt *a, *b;
    // if (left->nDigits > right->nDigits) {
    //     a = left;
    //     b = right;
    // } else {
    //     a = right;
    //     b = left;
    // }

    // digit_t *aMsdPtr = BI_MSDPTR(a);
    // digit_t *bMsdPtr = BI_MSDPTR(b);
    // digit_t *aPtr = BI_LSDPTR(a);
    // digit_t *bPtr = BI_LSDPTR(b);
    // digit_t *resPtr = BI_LSDPTR(result);
    // BigInt_Error_t eCode = E_SUCCESS;

    return E_FAILURE;
}

#endif  // BIGINT_H_