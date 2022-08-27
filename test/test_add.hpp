#ifndef TEST_ADD_H_
#define TEST_ADD_H_

#include "qtest.hpp"
#include "test_utils.hpp"

extern "C" {
    #include "../include/bigint.h"
}

QTEST_CASE(BigInt, AddBothPositive) {
    BigInt* bigInt42  = BigInt_FromString("42");
    BigInt* bigInt8   = BigInt_FromString("8");
    BigInt* bigIntSum = BigInt_New(0);

    QTEST_EXPECT(bigInt42->isNegative == false);
    QTEST_EXPECT(bigInt8->isNegative == false);

    QTEST_EXPECT(BigInt_Add(bigInt42, bigInt8, bigIntSum) == BIG_INT_E_SUCCESS);

    // check the summands dont change
    QTEST_EXPECT(bigInt42 != NULL);
    QTEST_EXPECT(bigInt42->capacity == 2);
    QTEST_EXPECT(bigInt42->nDigits == 2);
    QTEST_EXPECT(bigInt42->isNegative == false);
    QTEST_EXPECT(bigInt42->digits != NULL);
    QTEST_EXPECT(bigInt42->digits[0] == 2);
    QTEST_EXPECT(bigInt42->digits[1] == 4);

    QTEST_EXPECT(bigInt8 != NULL);
    QTEST_EXPECT(bigInt8->capacity == 1);
    QTEST_EXPECT(bigInt8->nDigits == 1);
    QTEST_EXPECT(bigInt8->isNegative == false);
    QTEST_EXPECT(bigInt8->digits != NULL);
    QTEST_EXPECT(bigInt8->digits[0] == 8);

    // check the result is correct
    QTEST_EXPECT(bigIntSum != NULL);
    QTEST_EXPECT(bigIntSum->capacity >= 2);
    QTEST_EXPECT(bigIntSum->nDigits == 2);
    QTEST_EXPECT(bigIntSum->isNegative == false);
    QTEST_EXPECT(bigIntSum->digits != NULL);
    QTEST_EXPECT(bigIntSum->digits[0] == 0);
    QTEST_EXPECT(bigIntSum->digits[1] == 5);

    BigInt_Free(bigInt42);
    BigInt_Free(bigInt8);
    BigInt_Free(bigIntSum);
}

QTEST_CASE(BigInt, AddTwoPositiveLong) {
    BigInt* bigInt1  = BigInt_FromString("999999999999999999999999999999999999");
    BigInt* bigInt2   = BigInt_FromString("999999999999999999999999999999999999");
    BigInt* bigIntSum = BigInt_New(0);

    QTEST_EXPECT(bigInt1->isNegative == false);
    QTEST_EXPECT(bigInt2->isNegative == false);

    QTEST_EXPECT(BigInt_Add(bigInt1, bigInt2, bigIntSum) == BIG_INT_E_SUCCESS);

    // check the summands dont change
    QTEST_EXPECT(bigInt1 != NULL);
    QTEST_EXPECT(bigInt1->capacity >= 36);
    QTEST_EXPECT(bigInt1->nDigits == 36);
    QTEST_EXPECT(bigInt1->isNegative == false);
    QTEST_EXPECT(bigInt1->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt1->digits, bigInt1->nDigits);

    QTEST_EXPECT(bigInt2 != NULL);
    QTEST_EXPECT(bigInt2->capacity >= 36);
    QTEST_EXPECT(bigInt2->nDigits == 36);
    QTEST_EXPECT(bigInt2->isNegative == false);
    QTEST_EXPECT(bigInt2->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt2->digits, bigInt2->nDigits);

    // check the result is correct
    QTEST_EXPECT(bigIntSum != NULL);
    QTEST_EXPECT(bigIntSum->capacity >= 37);
    QTEST_EXPECT(bigIntSum->nDigits == 37);
    QTEST_EXPECT(bigIntSum->isNegative == false);
    QTEST_EXPECT(bigIntSum->digits != NULL);
    compareArrays((digit_t[]) {8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,1}, bigIntSum->digits, bigIntSum->nDigits);

    BigInt_Free(bigInt1);
    BigInt_Free(bigInt2);
    BigInt_Free(bigIntSum);
}

QTEST_CASE(BigInt, AddPosAndNegLong) {
    BigInt* bigInt1  = BigInt_FromString("999999999999999999999999999999999999");
    BigInt* bigInt2   = BigInt_FromString("-9999999999999999999999999999999999999");
    BigInt* bigIntSum = BigInt_New(0);

    QTEST_EXPECT(bigInt1->isNegative == false);
    QTEST_EXPECT(bigInt2->isNegative == true);

    QTEST_EXPECT(BigInt_Add(bigInt1, bigInt2, bigIntSum) == BIG_INT_E_SUCCESS);

    // check the summands dont change
    QTEST_EXPECT(bigInt1 != NULL);
    QTEST_EXPECT(bigInt1->capacity >= 36);
    QTEST_EXPECT(bigInt1->nDigits == 36);
    QTEST_EXPECT(bigInt1->isNegative == false);
    QTEST_EXPECT(bigInt1->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt1->digits, bigInt1->nDigits);

    QTEST_EXPECT(bigInt2 != NULL);
    QTEST_EXPECT(bigInt2->capacity >= 37);
    QTEST_EXPECT(bigInt2->nDigits == 37);
    QTEST_EXPECT(bigInt2->isNegative == true);
    QTEST_EXPECT(bigInt2->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt2->digits, bigInt2->nDigits);

    // check the result is correct
    QTEST_EXPECT(bigIntSum != NULL);
    QTEST_EXPECT(bigIntSum->capacity >= 37);
    QTEST_EXPECT(bigIntSum->nDigits == 37);
    QTEST_EXPECT(bigIntSum->isNegative == true);
    QTEST_EXPECT(bigIntSum->digits != NULL);
    compareArrays((digit_t[]) {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9}, bigIntSum->digits, bigIntSum->nDigits);

    BigInt_Free(bigInt1);
    BigInt_Free(bigInt2);
    BigInt_Free(bigIntSum);

    bigInt1  = BigInt_FromString("-999999999999999999999999999999999999");
    bigInt2   = BigInt_FromString("9999999999999999999999999999999999999");
    bigIntSum = BigInt_New(0);

    QTEST_EXPECT(bigInt1->isNegative == true);
    QTEST_EXPECT(bigInt2->isNegative == false);

    QTEST_EXPECT(BigInt_Add(bigInt1, bigInt2, bigIntSum) == BIG_INT_E_SUCCESS);

    // check the summands dont change
    QTEST_EXPECT(bigInt1 != NULL);
    QTEST_EXPECT(bigInt1->capacity >= 36);
    QTEST_EXPECT(bigInt1->nDigits == 36);
    QTEST_EXPECT(bigInt1->isNegative == true);
    QTEST_EXPECT(bigInt1->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt1->digits, bigInt1->nDigits);

    QTEST_EXPECT(bigInt2 != NULL);
    QTEST_EXPECT(bigInt2->capacity >= 37);
    QTEST_EXPECT(bigInt2->nDigits == 37);
    QTEST_EXPECT(bigInt2->isNegative == false);
    QTEST_EXPECT(bigInt2->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt2->digits, bigInt2->nDigits);

    // check the result is correct
    QTEST_EXPECT(bigIntSum != NULL);
    QTEST_EXPECT(bigIntSum->capacity >= 37);
    QTEST_EXPECT(bigIntSum->nDigits == 37);
    QTEST_EXPECT(bigIntSum->isNegative == true);
    QTEST_EXPECT(bigIntSum->digits != NULL);
    compareArrays((digit_t[]) {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9}, bigIntSum->digits, bigIntSum->nDigits);

    BigInt_Free(bigInt1);
    BigInt_Free(bigInt2);
    BigInt_Free(bigIntSum);
}

QTEST_CASE(BigInt, AddBothNegativeLong) {
    BigInt* bigInt1  = BigInt_FromString("-999999999999999999999999999999999999");
    BigInt* bigInt2   = BigInt_FromString("-9999999999999999999999999999999999999");
    BigInt* bigIntSum = BigInt_New(0);

    QTEST_EXPECT(bigInt1->isNegative == true);
    QTEST_EXPECT(bigInt2->isNegative == true);

    QTEST_EXPECT(BigInt_Add(bigInt1, bigInt2, bigIntSum) == BIG_INT_E_SUCCESS);

    // check the summands dont change
    QTEST_EXPECT(bigInt1 != NULL);
    QTEST_EXPECT(bigInt1->capacity >= 36);
    QTEST_EXPECT(bigInt1->nDigits == 36);
    QTEST_EXPECT(bigInt1->isNegative == true);
    QTEST_EXPECT(bigInt1->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt1->digits, bigInt1->nDigits);

    QTEST_EXPECT(bigInt2 != NULL);
    QTEST_EXPECT(bigInt2->capacity >= 37);
    QTEST_EXPECT(bigInt2->nDigits == 37);
    QTEST_EXPECT(bigInt2->isNegative == true);
    QTEST_EXPECT(bigInt2->digits != NULL);
    compareArrays((digit_t[]) {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}, bigInt2->digits, bigInt2->nDigits);

    // check the result is correct
    QTEST_EXPECT(bigIntSum != NULL);
    QTEST_EXPECT(bigIntSum->capacity >= 38);
    QTEST_EXPECT(bigIntSum->nDigits == 38);
    QTEST_EXPECT(bigIntSum->isNegative == true);
    QTEST_EXPECT(bigIntSum->digits != NULL);
    compareArrays((digit_t[]) {8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,0,1}, bigIntSum->digits, bigIntSum->nDigits);

    BigInt_Free(bigInt1);
    BigInt_Free(bigInt2);
    BigInt_Free(bigIntSum);
}

#endif  // TEST_ADD_H_