extern "C" {
    // #include "../include/crypt.h"
    #include "../include/bigint.h"
}

#include "qtest.hpp"

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
    QTEST_EXPECT(bigIntPos->digits[0] == 2);
    QTEST_EXPECT(bigIntPos->digits[1] == 4);
    BigInt_Free(bigIntPos);

    BigInt* bigIntNeg = BigInt_FromString("-42");
    QTEST_EXPECT(bigIntNeg != NULL);
    QTEST_EXPECT(bigIntNeg->isNegative == true);
    QTEST_EXPECT(bigIntNeg->nDigits == 2);
    QTEST_EXPECT(bigIntNeg->capacity == 2);
    QTEST_EXPECT(bigIntNeg->digits != NULL);
    QTEST_EXPECT(bigIntPos->digits[0] == 2);
    QTEST_EXPECT(bigIntPos->digits[1] == 4);
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

QTEST_CASE(BigInt, Add) {
    BigInt* bigInt42  = BigInt_Init(42);
    BigInt* bigInt8   = BigInt_Init(8);
    BigInt* bigIntSum = BigInt_Init(0);

    QTEST_EXPECT(BigInt_Add(bigInt42, bigInt8, bigIntSum) == E_SUCCESS);

    BigInt_Free(bigInt42);
    BigInt_Free(bigInt8);
    BigInt_Free(bigIntSum);
}

int main(int argc, char const *argv[]) {
    QTEST_RUN_ALL();
    return 0;
}
