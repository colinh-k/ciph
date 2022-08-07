extern "C" {
    #include "../include/bigint.h"
}

#include "qtest.hpp"

// helper function to make assersions about the contents
// of digit_t arrays
void compareArrays(digit_t expected[], digit_t actual[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        QTEST_EXPECT(expected[i] == actual[i]);
    }
}

QTEST_CASE(BigInt, Init) {
    BigInt* bigInt = BigInt_Init(42);
    QTEST_EXPECT(bigInt != NULL);
    QTEST_EXPECT(bigInt->isNegative == false);
    QTEST_EXPECT(bigInt->nDigits == 0);
    QTEST_EXPECT(bigInt->capacity == 42);
    QTEST_EXPECT(bigInt->digits != NULL);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, FromString) {
    BigInt* bigIntPos = BigInt_FromString("42");
    QTEST_EXPECT(bigIntPos != NULL);
    QTEST_EXPECT(bigIntPos->isNegative == false);
    QTEST_EXPECT(bigIntPos->nDigits == 2);
    QTEST_EXPECT(bigIntPos->capacity == 2);
    QTEST_EXPECT(bigIntPos->digits != NULL);
    // QTEST_EXPECT(bigIntPos->digits[0] == 2);
    // QTEST_EXPECT(bigIntPos->digits[1] == 4);
    compareArrays((digit_t[]) {2, 4}, bigIntPos->digits, 2);
    BigInt_Free(bigIntPos);

    BigInt* bigIntNeg = BigInt_FromString("-42");
    QTEST_EXPECT(bigIntNeg != NULL);
    QTEST_EXPECT(bigIntNeg->isNegative == true);
    QTEST_EXPECT(bigIntNeg->nDigits == 2);
    QTEST_EXPECT(bigIntNeg->capacity == 2);
    QTEST_EXPECT(bigIntNeg->digits != NULL);
    QTEST_EXPECT(bigIntNeg->digits[0] == 2);
    QTEST_EXPECT(bigIntNeg->digits[1] == 4);
    BigInt_Free(bigIntNeg);
}

QTEST_CASE(BigInt, FromStringLong) {
    BigInt* bigIntLong = BigInt_FromString("123456789101112131415");
    QTEST_EXPECT(bigIntLong != NULL);
    QTEST_EXPECT(bigIntLong->isNegative == false);
    QTEST_EXPECT(bigIntLong->nDigits == 21);
    QTEST_EXPECT(bigIntLong->capacity == 21);
    QTEST_EXPECT(bigIntLong->digits != NULL);
    QTEST_EXPECT(bigIntLong->digits[0] == 5);
    QTEST_EXPECT(bigIntLong->digits[1] == 1);
    QTEST_EXPECT(bigIntLong->digits[2] == 4);
    QTEST_EXPECT(bigIntLong->digits[3] == 1);
    QTEST_EXPECT(bigIntLong->digits[4] == 3);
    QTEST_EXPECT(bigIntLong->digits[5] == 1);
    QTEST_EXPECT(bigIntLong->digits[6] == 2);
    QTEST_EXPECT(bigIntLong->digits[7] == 1);
    QTEST_EXPECT(bigIntLong->digits[8] == 1);
    QTEST_EXPECT(bigIntLong->digits[9] == 1);
    QTEST_EXPECT(bigIntLong->digits[10] == 0);
    QTEST_EXPECT(bigIntLong->digits[11] == 1);
    QTEST_EXPECT(bigIntLong->digits[12] == 9);
    QTEST_EXPECT(bigIntLong->digits[13] == 8);
    QTEST_EXPECT(bigIntLong->digits[14] == 7);
    QTEST_EXPECT(bigIntLong->digits[15] == 6);
    QTEST_EXPECT(bigIntLong->digits[16] == 5);
    QTEST_EXPECT(bigIntLong->digits[17] == 4);
    QTEST_EXPECT(bigIntLong->digits[18] == 3);
    QTEST_EXPECT(bigIntLong->digits[19] == 2);
    QTEST_EXPECT(bigIntLong->digits[20] == 1);
    BigInt_Free(bigIntLong);
}

QTEST_CASE(BigInt, FromStringError) {
    QTEST_EXPECT(BigInt_FromString("not a number") == NULL);
    QTEST_EXPECT(BigInt_FromString("-not a number") == NULL);
    QTEST_EXPECT(BigInt_FromString("1abc") == NULL);
    QTEST_EXPECT(BigInt_FromString("-1abc") == NULL);
    QTEST_EXPECT(BigInt_FromString("a") == NULL);
    QTEST_EXPECT(BigInt_FromString("-a") == NULL);
    QTEST_EXPECT(BigInt_FromString("23984765283476a23894569") == NULL);
    QTEST_EXPECT(BigInt_FromString("-23984765283476a23894569") == NULL);
    QTEST_EXPECT(BigInt_FromString("2398476528347623894569a") == NULL);
    QTEST_EXPECT(BigInt_FromString("-2398476528347623894569a") == NULL);
}

QTEST_CASE(BigInt, ToString) {
    BigInt* bigInt = BigInt_FromString("4266");
    char* bigIntStr = BigInt_ToString(bigInt);
    QTEST_EXPECT(bigIntStr != NULL);
    QTEST_EXPECT(strcmp(bigIntStr, "4266") == 0);
    free(bigIntStr);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, ResizeBigger) {
    BigInt* bigInt = BigInt_FromString("456");
    QTEST_EXPECT(BigInt_Resize(bigInt, 10) == BIG_INT_E_SUCCESS);
    QTEST_EXPECT(bigInt != NULL);
    QTEST_EXPECT(bigInt->isNegative == false);
    QTEST_EXPECT(bigInt->nDigits == 3);
    QTEST_EXPECT(bigInt->capacity == 10);
    QTEST_EXPECT(bigInt->digits != NULL);
    QTEST_EXPECT(bigInt->digits[0] == 6);
    QTEST_EXPECT(bigInt->digits[1] == 5);
    QTEST_EXPECT(bigInt->digits[2] == 4);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, ResizeSmaller) {
    BigInt* bigInt = BigInt_FromString("456");
    QTEST_EXPECT(BigInt_Resize(bigInt, 1) == BIG_INT_E_SUCCESS);
    QTEST_EXPECT(bigInt != NULL);
    QTEST_EXPECT(bigInt->isNegative == false);
    QTEST_EXPECT(bigInt->nDigits == 1);
    QTEST_EXPECT(bigInt->capacity == 1);
    QTEST_EXPECT(bigInt->digits != NULL);
    QTEST_EXPECT(bigInt->digits[0] == 6);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, ResizeSame) {
    BigInt* bigInt = BigInt_FromString("456");
    QTEST_EXPECT(BigInt_Resize(bigInt, 3) == BIG_INT_E_SUCCESS);
    QTEST_EXPECT(bigInt != NULL);
    QTEST_EXPECT(bigInt->isNegative == false);
    QTEST_EXPECT(bigInt->nDigits == 3);
    QTEST_EXPECT(bigInt->capacity == 3);
    QTEST_EXPECT(bigInt->digits != NULL);
    QTEST_EXPECT(bigInt->digits[0] == 6);
    QTEST_EXPECT(bigInt->digits[1] == 5);
    QTEST_EXPECT(bigInt->digits[2] == 4);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, AddSimple) {
    BigInt* bigInt42  = BigInt_FromString("42");
    BigInt* bigInt8   = BigInt_FromString("8");
    BigInt* bigIntSum = BigInt_Init(0);

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
    QTEST_EXPECT(bigIntSum->capacity == 2);
    QTEST_EXPECT(bigIntSum->nDigits == 2);
    QTEST_EXPECT(bigIntSum->isNegative == false);
    QTEST_EXPECT(bigIntSum->digits != NULL);
    QTEST_EXPECT(bigIntSum->digits[0] == 0);
    QTEST_EXPECT(bigIntSum->digits[1] == 5);

    BigInt_Free(bigInt42);
    BigInt_Free(bigInt8);
    BigInt_Free(bigIntSum);
}

int main(int argc, char const *argv[]) {
    QTEST_RUN_ALL();
    return 0;
}
