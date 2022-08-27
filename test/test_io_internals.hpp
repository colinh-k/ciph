#ifndef TEST_IO_INTERNALS_H_
#define TEST_IO_INTERNALS_H_

#include "qtest.hpp"
#include "test_utils.hpp"

extern "C" {
    #include "../include/bigint.h"
}

QTEST_CASE(BigInt, New) {
    BigInt* bigInt = BigInt_New(42);
    BIG_INT_EXPECT_CAP(bigInt, false, 0, 42);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, FromString) {
    BigInt* bigIntPos = BigInt_FromString("42");
    BIG_INT_EXPECT(bigIntPos, false, 2);
    BIG_INT_EXPECT_DIGITS(bigIntPos, ({2, 4}));
    BigInt_Free(bigIntPos);

    BigInt* bigIntNeg = BigInt_FromString("-42");
    BIG_INT_EXPECT(bigIntNeg, true, 2);
    BIG_INT_EXPECT_DIGITS(bigIntNeg, ({2, 4}));
    BigInt_Free(bigIntNeg);
}

QTEST_CASE(BigInt, FromStringLong) {
    BigInt* bigint = BigInt_FromString("123456789101112131415");
    BIG_INT_EXPECT(bigint, false, 21);
    BIG_INT_EXPECT_DIGITS(bigint, ({5,1,4,1,3,1,2,1,1,1,0,1,9,8,7,6,5,4,3,2,1}));
    BigInt_Free(bigint);

    bigint = BigInt_FromString("-123456789101112131415");
    BIG_INT_EXPECT(bigint, true, 21);
    BIG_INT_EXPECT_DIGITS(bigint, ({5,1,4,1,3,1,2,1,1,1,0,1,9,8,7,6,5,4,3,2,1}));
    BigInt_Free(bigint);
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
    QTEST_EXPECT(BigInt_FromString("--1") == NULL);
    QTEST_EXPECT(BigInt_FromString("--1") == NULL);
    QTEST_EXPECT(BigInt_FromString("1-") == NULL);
    QTEST_EXPECT(BigInt_FromString("1--") == NULL);
    QTEST_EXPECT(BigInt_FromString("-1-") == NULL);
    QTEST_EXPECT(BigInt_FromString(" 1") == NULL);
    QTEST_EXPECT(BigInt_FromString("1 ") == NULL);
    QTEST_EXPECT(BigInt_FromString(" 1 ") == NULL);
    QTEST_EXPECT(BigInt_FromString("-1239176-") == NULL);
    QTEST_EXPECT(BigInt_FromString(" 1239176") == NULL);
    QTEST_EXPECT(BigInt_FromString("1239176 ") == NULL);
    QTEST_EXPECT(BigInt_FromString(" 1239176 ") == NULL);
}

QTEST_CASE(BigInt, ToString) {
    BigInt* bigInt = BigInt_FromString("4266");
    char* bigIntStr = BigInt_ToString(bigInt);
    QTEST_EXPECT(bigIntStr != NULL);
    QTEST_EXPECT_EQUALS(0, strcmp(bigIntStr, "4266"));
    // QTEST_EXPECT(strcmp(bigIntStr, "4266") == 0);
    free(bigIntStr);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, ResizeBigger) {
    BigInt* bigInt = BigInt_FromString("456");
    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Resize(bigInt, 10));
    // QTEST_EXPECT(BigInt_Resize(bigInt, 10) == BIG_INT_E_SUCCESS);
    BIG_INT_EXPECT_CAP(bigInt, false, 3, 10);
    BIG_INT_EXPECT_DIGITS(bigInt, ({6,5,4}));

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
    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Resize(bigInt, 1));
    // QTEST_EXPECT(BigInt_Resize(bigInt, 1) == BIG_INT_E_SUCCESS);
    BIG_INT_EXPECT(bigInt, false, 1);
    QTEST_EXPECT(bigInt->digits[0] == 6);
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, ResizeSame) {
    BigInt* bigInt = BigInt_FromString("456");
    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Resize(bigInt, 3));
    // QTEST_EXPECT(BigInt_Resize(bigInt, 3) == BIG_INT_E_SUCCESS);
    BIG_INT_EXPECT(bigInt, false, 3);
    BIG_INT_EXPECT_DIGITS(bigInt, ({6,5,4}));
    BigInt_Free(bigInt);
}

QTEST_CASE(BigInt, Compare) {
    BigInt* bigIntBigger = BigInt_FromString("2");
    BigInt* bigIntSmaller = BigInt_FromString("1");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("-1");
    bigIntSmaller = BigInt_FromString("-2");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("12345");
    bigIntSmaller = BigInt_FromString("1234");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("12345");
    bigIntSmaller = BigInt_FromString("12344");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("55555");
    bigIntSmaller = BigInt_FromString("55554");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("-1234");
    bigIntSmaller = BigInt_FromString("-12345");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("-12344");
    bigIntSmaller = BigInt_FromString("-12345");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("-55554");
    bigIntSmaller = BigInt_FromString("-55555");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_GT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_LT);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
    bigIntBigger = BigInt_FromString("-2");
    bigIntSmaller = BigInt_FromString("-2");
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntBigger) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntSmaller, bigIntSmaller) == BIG_INT_EQ);
    QTEST_EXPECT(BigInt_Compare(bigIntBigger, bigIntBigger) == BIG_INT_EQ);
    BigInt_Free(bigIntBigger);
    BigInt_Free(bigIntSmaller);
}

#endif  // TEST_IO_INTERNALS_H_