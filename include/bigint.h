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
    BIG_INT_E_SUCCESS,
    BIG_INT_E_FAILURE,
    BIG_INT_E_OVERFLOW,
    BIG_INT_E_UNDERFLOW,
    BIG_INT_E_DIVISION_BY_ZERO,  // divizion by zero
    BIG_INT_E_MEMORY_ALLOCATION,  // memory allocation error
    BIG_INT_E_MODULUS,  // Montgomery multiplication given even modulus
    BIG_INT_E_NULL_ARGUMENT  // NULL passed as argument
} BigInt_Error_t;

typedef struct {
    // TODO: is it necessary to store the code in this struct ?
    // we can just map from the error code to the index in the
    // string arrary
    BigInt_Error_t eCode;
    const char* msg;
} BigInt_ErrorDesc_t;

BigInt_ErrorDesc_t eDesc[] = {
    { BIG_INT_E_SUCCESS, "operation successful" },
    { BIG_INT_E_FAILURE, "operation failure" },
    { BIG_INT_E_OVERFLOW, "operation resulted in overflow" },
    { BIG_INT_E_UNDERFLOW, "operation resulted in underflow" },
    { BIG_INT_E_DIVISION_BY_ZERO, "operation attempted division by zero" },
    { BIG_INT_E_MEMORY_ALLOCATION, "memory allocation failed" },
    { BIG_INT_E_MODULUS, "Montgomery multiplication was given an even modulus" },
    { BIG_INT_E_NULL_ARGUMENT, "operation was given a NULL pointer argument" }
};

void BigInt_PrintError(BigInt_Error_t code) {
    fprintf(stderr, "%s\n", eDesc[code].msg);
}

BigInt* BigInt_Init(size_t capacity);
BigInt* BigInt_FromString(const char* str);
// BigInt* BigInt_Copy(BigInt* other);
// int8_t  BigInt_Compare(BigInt* other);
void BigInt_Free(BigInt* bigInt);
// helper functions
char* BigInt_ToString(BigInt *bigInt);
BigInt_Error_t BigInt_Resize(BigInt *bigInt, size_t newCapacity);
BigInt_Error_t BigInt_Add(BigInt* left, BigInt* right, BigInt* result);

BigInt* BigInt_Init(size_t capacity) {
    BigInt *ret = (BigInt*) malloc(sizeof(BigInt));
    if (!ret)
        return NULL;
    ret->isNegative = false;
    ret->capacity = capacity;
    ret->nDigits = 0;
    ret->digits = (digit_t*) malloc(sizeof(digit_t) * capacity);
    if (!(ret->digits)) {
        free(ret);  // first allocation (ret) succeeded, so free it before return
        return NULL;
    }
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
    // for (size_t i = nDigits - 1; i != (size_t) -1; i--) {
    for (size_t i = 0; i < nDigits; i++) {
        char c = str[nDigits - 1 - i];
        if (isdigit(c)) {
            ret->digits[i] = (digit_t) (c - '0');
        } else {
            // invalid character for integer type (i.e., NaN)
            BigInt_Free(ret);  // REMEMBER TO FREE ALLOCATED MEMORY
            return NULL;
        }
    }
    return ret;
}

void BigInt_Free(BigInt* bigint) {
    free(bigint->digits);
    free(bigint);
}

// NOTE: caller is responsible for free'ing the allocated
// buffer that is returned
char* BigInt_ToString(BigInt *bigInt) {
    // the length of the string is the number of digits, plus 1 for '-'
    // prefix if it is negative
    size_t strLen = (bigInt->isNegative) ? bigInt->nDigits + 1 : bigInt->nDigits;
    char* str = (char*) malloc(strLen * sizeof(char));

    if (!str)
        return NULL;

    for (size_t i = 0; i < strLen; i++) {
        str[i] = (char) (bigInt->digits[strLen - 1 - i] + '0');
    }

    return str;
}

BigInt_Error_t BigInt_Resize(BigInt *bigInt, size_t newCapacity) {
    if (newCapacity <= bigInt->capacity) {
        bigInt->nDigits = (newCapacity > bigInt->nDigits) ? bigInt->nDigits : newCapacity;
    } else {
        bigInt->digits = (digit_t*) realloc(bigInt->digits, newCapacity);
        if (!(bigInt->digits)) {
            return BIG_INT_E_MEMORY_ALLOCATION;
        }
    }
    bigInt->capacity = newCapacity;
    return BIG_INT_E_SUCCESS;
}

BigInt_Error_t BigInt_Add(BigInt* left, BigInt* right, BigInt* result) {
    if (left == NULL || right == NULL || result == NULL)
        return BIG_INT_E_NULL_ARGUMENT;

    // 1. init ptrs so a points to longer int or a, and b
    // points to shorter int or b
    BigInt *a, *b;
    if (left->nDigits > right->nDigits) {
        a = left;
        b = right;
    } else {
        a = right;
        b = left;
    }

    // 1.5. set the sign of the result

    // 2. resize the result if necessary
    BigInt_Resize(result, a->capacity);
    result->nDigits = a->nDigits;

    size_t i = 0;
    digit_t carry = 0;
    // 3. loop over the shorter int from lsd to msd
    for (; i < b->nDigits; i++) {
        digit_t t_sum = b->digits[i] + a->digits[i] + carry;
        carry = t_sum / 10;  // TODO: divide by the base instead of 10 ?
        result->digits[i] = t_sum % 10;
    }

    // 4. loop over the longer int to move those digits to the result
    // accounting for any carry
    for (; i < a->nDigits; i++) {
        digit_t t_sum = a->digits[i] + carry;
        carry = t_sum / 10;  // TODO: divide by the base instead of 10 ?
        result->digits[i] = t_sum % 10;  // same as above here ?
    }

    // 5. check if we need to append a final carry digit to the end of the result int
    if (carry) {
        BigInt_Resize(result, result->capacity + 1);
        result->digits[result->capacity - 1] = carry;
        (result->nDigits)++;
    }

    return BIG_INT_E_SUCCESS;
}

#endif  // BIGINT_H_