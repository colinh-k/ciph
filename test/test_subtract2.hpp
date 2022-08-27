#ifndef TEST_SUBTRACT_2_H_
#define TEST_SUBTRACT_2_H_

#include "qtest.hpp"
#include "test_utils.hpp"

extern "C" {
    #include "../include/bigint.h"
}

QTEST_CASE(BigInt, SubtractTwoPositive) {
    BigInt* biA  = BigInt_FromString("9759");  // a
    BigInt* biB  = BigInt_FromString("327");   // b
    BigInt* biS  = BigInt_Zero();             // sum (should be 10086)

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biA, biB, biS));
    BIG_INT_EXPECT(biA, false, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, false, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, false, 4);
    BIG_INT_EXPECT_DIGITS(biS, ({2, 3, 4, 9}));

    BigInt_Free(biS);  // free then reset the result struct
    biS = BigInt_Zero();

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biB, biA, biS));
    BIG_INT_EXPECT(biA, false, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, false, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, true, 4);
    BIG_INT_EXPECT_DIGITS(biS, ({2, 3, 4, 9}));

    BigInt_Free(biA);
    BigInt_Free(biB);
    BigInt_Free(biS);
}

QTEST_CASE(BigInt, SubtractTwoNegative) {
    BigInt* biA  = BigInt_FromString("-9759");  // a
    BigInt* biB  = BigInt_FromString("-327");   // b
    BigInt* biS  = BigInt_Zero();             // sum (should be -10086)

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biA, biB, biS));
    BIG_INT_EXPECT(biA, true, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, true, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, true, 4);
    BIG_INT_EXPECT_DIGITS(biS, ({2, 3, 4, 9}));

    BigInt_Free(biS);  // free then reset the result struct
    biS = BigInt_Zero();

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biB, biA, biS));
    BIG_INT_EXPECT(biA, true, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, true, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, false, 4);
    BIG_INT_EXPECT_DIGITS(biS, ({2, 3, 4, 9}));

    BigInt_Free(biA);
    BigInt_Free(biB);
    BigInt_Free(biS);
}

QTEST_CASE(BigInt, SubtractOppositeSigns) {
    BigInt* biA  = BigInt_FromString("-9759");  // a
    BigInt* biB  = BigInt_FromString("327");   // b
    BigInt* biS  = BigInt_Zero();             // (should be -10086)

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biA, biB, biS));
    BIG_INT_EXPECT(biA, true, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, false, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, true, 5);
    BIG_INT_EXPECT_DIGITS(biS, ({6, 8, 0, 0, 1}));

    BigInt_Free(biS);  // free then reset the result struct
    biS = BigInt_Zero();

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biB, biA, biS));
    BIG_INT_EXPECT(biA, true, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, false, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, false, 5);
    BIG_INT_EXPECT_DIGITS(biS, ({6, 8, 0, 0, 1}));

    BigInt_Free(biA);
    BigInt_Free(biB);
    BigInt_Free(biS);

    biA = BigInt_FromString("9759");  // a
    biB = BigInt_FromString("-327");  // b
    biS = BigInt_Zero();              // sum (should be 9432)

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biA, biB, biS));
    BIG_INT_EXPECT(biA, false, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, true, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, false, 5);
    BIG_INT_EXPECT_DIGITS(biS, ({6, 8, 0, 0, 1}));

    BigInt_Free(biS);  // free then reset the result struct
    biS = BigInt_Zero();

    QTEST_EXPECT_EQUALS(BIG_INT_E_SUCCESS, BigInt_Subtract(biB, biA, biS));
    BIG_INT_EXPECT(biA, false, 4);  // check operands dont change after operation
    BIG_INT_EXPECT_DIGITS(biA, ({9, 5, 7, 9}));
    BIG_INT_EXPECT(biB, true, 3);
    BIG_INT_EXPECT_DIGITS(biB, ({7, 2, 3}));
    BIG_INT_EXPECT(biS, true, 5);
    BIG_INT_EXPECT_DIGITS(biS, ({6, 8, 0, 0, 1}));

    BigInt_Free(biA);
    BigInt_Free(biB);
    BigInt_Free(biS);
}

#endif  // TEST_SUBTRACT_2_H_