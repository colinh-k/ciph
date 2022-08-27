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
typedef uint16_t result_t;


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
    BIG_INT_LT = -1,
    BIG_INT_EQ =  0,
    BIG_INT_GT =  1
} BigInt_Compare_t;

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

BigInt* BigInt_New(size_t capacity);
BigInt* BigInt_Zero(void);
BigInt* BigInt_FromString(const char* str);
// BigInt* BigInt_Copy(BigInt* other);
// int8_t  BigInt_Compare(BigInt* other);
void BigInt_Free(BigInt* bigInt);
// helper functions
char* BigInt_ToString(BigInt *bigInt);
BigInt_Compare_t BigInt_Compare(BigInt* left, BigInt* right);
BigInt_Error_t BigInt_Resize(BigInt *bigInt, size_t newCapacity);
BigInt_Error_t BigInt_Add(BigInt* left, BigInt* right, BigInt* result);
BigInt_Error_t BigInt_Subtract(BigInt* left, BigInt* right, BigInt* result);

BigInt_Error_t BigInt_Multiply(BigInt* left, BigInt* right, BigInt* result);
BigInt_Error_t BigInt_Divide(BigInt* left, BigInt* right, BigInt* result);

BigInt_Error_t BigInt_SetZero(BigInt* bigint);
BigInt_Error_t BigInt_IsZero(BigInt* bigint);
void BigInt_TrimZeros(BigInt* bigint);

BigInt* BigInt_New(size_t capacity) {
    if (capacity == 0) {
        return BigInt_Zero();
    }

    BigInt *ret = (BigInt*) malloc(sizeof(BigInt));
    if (!ret) {
        return NULL;
    }

    ret->isNegative = false;
    ret->nDigits = 0;
    ret->capacity = capacity;
    ret->digits = (digit_t*) malloc(sizeof(digit_t) * capacity);

    if (!(ret->digits)) {
        free(ret);  // first allocation (ret) succeeded, so free it before return
        return NULL;
    }
    return ret;
}

BigInt* BigInt_Zero(void) {
    BigInt* ret = BigInt_New(1);
    if (!ret) {
        return NULL;
    }
    ret->nDigits = 1;
    ret->digits[0] = 0;
    return ret;
}

BigInt* BigInt_FromString(const char* str) {
    bool isNegative = false;
    if (str[0] == '-') {
        isNegative = true;
        str++;  // ignore the leading dash
    }

    size_t nDigits = strlen(str);
    BigInt *ret = BigInt_New(nDigits);
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
    if (!bigint) {
        return;
    }
    if (bigint->digits) {
        free(bigint->digits);
    }
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

    if (left->isNegative && !(right->isNegative)) {
        // -a + b == b - a => sub(b, a)
        left->isNegative = false;
        BigInt_Error_t res = BigInt_Subtract(right, left, result);
        left->isNegative = true;
        return res;
    } else if (!(left->isNegative) && right->isNegative) {
        // a + -b == a - b => sub(a, b)
        right->isNegative = false;
        BigInt_Error_t res = BigInt_Subtract(left, right, result);
        right->isNegative = true;
        return res;
    } else if (left->isNegative && right->isNegative) {
        // -a + -b == - (a + b) => -add(a, b)
        left->isNegative = false;
        right->isNegative = false;
        BigInt_Error_t ret = BigInt_Add(left, right, result);
        left->isNegative = true;
        right->isNegative = true;
        result->isNegative = true;  // result must be negative
        return ret;
    }

    // a + b

    // 1. init ptrs so a points to longer int or a, and b
    // points to shorter int or b
    BigInt *a, *b;
    if (left->nDigits >= right->nDigits) {
        a = left;
        b = right;
    } else {
        a = right;
        b = left;
    }

    // 2. resize the result if necessary. +1 for potential carry on the MSD
    BigInt_Resize(result, a->nDigits + 1);
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
        // BigInt_Resize(result, result->capacity + 1);  // this should be taken care of in the resizing above
        result->digits[result->capacity - 1] = carry;
        (result->nDigits)++;
    }

    BigInt_TrimZeros(result);


    return BIG_INT_E_SUCCESS;
}

BigInt_Error_t BigInt_Subtract(BigInt* left, BigInt* right, BigInt* result) {
    if (left == NULL || right == NULL || result == NULL)
        return BIG_INT_E_NULL_ARGUMENT;

    if (left->isNegative && !(right->isNegative)) {
        // -a - b == - (a + b) => -add(a, b)
        left->isNegative = false;
        BigInt_Error_t res = BigInt_Add(left, right, result);
        left->isNegative = true;
        result->isNegative = true;
        return res;
    } else if (left->isNegative && right->isNegative) {
        // -a - -b == -a + b == b - a => sub(b, a)
        right->isNegative = false;
        left->isNegative = false;
        BigInt_Error_t res = BigInt_Subtract(right, left, result);
        right->isNegative = true;
        left->isNegative = true;
        return res;
    } else if (!(left->isNegative) && right->isNegative) {
        // a - -b == a + b => add(a, b)
        right->isNegative = false;
        BigInt_Error_t res = BigInt_Add(left, right, result);
        right->isNegative = true;
        return res;
    }
    // a - b:

    BigInt_Compare_t cmp = BigInt_Compare(left, right);
    if (cmp == BIG_INT_EQ) {
        result->nDigits = 1;
        result->digits[0] = 0;
        result->isNegative = false;
        return BIG_INT_E_SUCCESS;
    } else if (cmp == BIG_INT_LT) {
        // left < right => left - right => -(right - left)
        BigInt_Error_t res = BigInt_Subtract(right, left, result);
        result->isNegative = true;
        return res;
    }

    // left is bigger than right

    // 2. resize the result if necessary
    BigInt_Resize(result, left->nDigits);
    result->nDigits = left->nDigits;  // TODO: set result.nDigits after the computation ?

    bool carry = false;
    for (size_t i = 0; i < result->nDigits; i++) {
        bool t_carry = carry;
        digit_t t_dif;
        if (i < right->nDigits) {
            // right is guaranteed smaller, so left is at least as long as right
            if (left->digits[i] >= right->digits[i]) {  // result is positive (no underflow)
                t_dif = left->digits[i] - right->digits[i];
                carry = false;
            } else {  // set the carry
                t_dif = 10 + left->digits[i] - right->digits[i];
                carry = true;
            }
        } else {  // guaranteed [i < left->nDigits] (look at result->nDigits assignment above)
            t_dif = left->digits[i];
            carry = false;
        }
        // handle the carry
        if (t_carry) {
            t_dif -= 1;
        }
        result->digits[i] = t_dif;
    }

    BigInt_TrimZeros(result);

    return BIG_INT_E_SUCCESS;
}

// compares cmpFrom TO cmpTO; i.e., evaluates [cmpFrom (>,<,==) cmpTo]
BigInt_Compare_t BigInt_Compare(BigInt* cmpFrom, BigInt* cmpTo) {
    // if (cmpFrom == NULL || cmpTo == NULL) {
    //     return BIG_INT_E_NULL_ARGUMENT;
    // }
    if (!(cmpFrom->isNegative) && cmpTo->isNegative) {
        // a > b
        return BIG_INT_GT;
    } else if (cmpFrom->isNegative && !(cmpTo->isNegative)) {
        // a < b
        return BIG_INT_LT;
    }
    // TODO: ALTERNATIVE METHOD:
    //  if both are negative, just flip the sign and call BigInt_Compare
    //  then flip the signs again (to their original state). also
    //  negate the return value
    // else, both are equal sign
    // resGT: return when a comparison would yeild a gt
    // resLT: return when a comparison would yeild a lt
    BigInt_Compare_t resGT = BIG_INT_GT, resLT = BIG_INT_LT;
    if (cmpFrom->isNegative && cmpTo->isNegative) {
        // flip the comparison result, since we take
        // a larger magnitude negative number to be
        // smaller, vice versa
        resGT = BIG_INT_LT;
        resLT = BIG_INT_GT;
    }

    if (cmpFrom->nDigits > cmpTo->nDigits) {
        // a is longer, so is larger
        return resGT;
    } else if (cmpFrom->nDigits < cmpTo->nDigits) {
        // a is shorter, so is smaller
        return resLT;
    }

    // else, both have equal number of digits, so compare each digit
    // from largest magnitude to smallest magnitude
    for (size_t i = cmpFrom->nDigits - 1; i != (size_t) -1; i--) {
        // TODO: will this loop always iterate backwards though ALL of
        // the elements ? im concerned about the 'i != (size_t) -1'
        if (cmpFrom->digits[i] < cmpTo->digits[i]) {
            return resLT;
        } else if (cmpFrom->digits[i] > cmpTo->digits[i]) {
            return resGT;
        }
    }
    // all digits are equal, so the numbers are equal
    return BIG_INT_EQ;
}

void BigInt_TrimZeros(BigInt* bigint) {
    // trims any leading zeros (starting at the highest idx of the array).
    // leaves at least 1 digit, which may be 0
    // e.g., [0, 0, 1, 0] -> [0, 0, 1] (100 in decimal)
    //       [0, 0, 0] -> [0]
    //       [1, 0, 0] -> [1]
    while (bigint->nDigits != 1 && bigint->digits[bigint->nDigits - 1] == 0) {
        printf("%hhu [%zu]\n", bigint->digits[bigint->nDigits - 1], bigint->nDigits);
        bigint->nDigits--;
    }
}

BigInt_Error_t BigInt_Multiply(BigInt* left, BigInt* right, BigInt* result) {
    if (left == NULL || right == NULL || result == NULL) {
        return BIG_INT_E_NULL_ARGUMENT;
    }

    // if either operands are 0, result is 0
    BigInt* zero = BigInt_Zero();
    if (BigInt_Compare(left, zero) == BIG_INT_EQ ||
        BigInt_Compare(right, zero) == BIG_INT_EQ) {
        BigInt_SetZero(result);
        BigInt_Free(zero);
        return BIG_INT_E_SUCCESS;
    }
    BigInt_Free(zero);

    result->isNegative = left->isNegative ^ right->isNegative;
    BigInt_Resize(result, left->nDigits + right->nDigits);
    // zero out the result array
    bzero(result->digits, result->nDigits);
    result->nDigits = left->nDigits + right->nDigits;

    BigInt* a = left;
    BigInt* b = right;

    for (size_t i = 0; i < a->nDigits; i++) {
        digit_t carry = 0;
        for (size_t j = 0; j < b->nDigits; j++) {
            result_t uv = a->digits[i] * b->digits[j] + result->digits[i + j] + carry;
            result->digits[i + j] = (uv % 10);
            carry = uv / 10;
        }
        result->digits[i + b->nDigits] += carry;
    }

    BigInt_TrimZeros(result);
    return BIG_INT_E_SUCCESS;
}

BigInt_Error_t BigInt_Divide(BigInt* left, BigInt* right, BigInt* result) {
    if (left == NULL || right == NULL || result == NULL) {
        return BIG_INT_E_NULL_ARGUMENT;
    }
    BigInt_TrimZeros(result);
    return BIG_INT_E_FAILURE;
}

BigInt_Error_t BigInt_SetZero(BigInt* bigint) {
    if (bigint == NULL) {
        return BIG_INT_E_NULL_ARGUMENT;
    }

    BigInt_Error_t ret = BigInt_Resize(bigint, 1);
    if (ret != BIG_INT_E_SUCCESS) {
        return ret;
    }

    bigint->nDigits = 1;  // TODO: is this already set in bigint_resize ?
    bigint->digits[0] = 0;
    return BIG_INT_E_SUCCESS;
}

#endif  // BIGINT_H_