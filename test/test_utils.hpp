#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <stdarg.h>  // for _VA_ARGS_

// put generic helper functions in here

// helper function to make assersions about the contents
// of digit_t arrays
void compareArrays(digit_t expected[], digit_t actual[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        QTEST_EXPECT(expected[i] == actual[i]);
    }
}

#define BIG_INT_EXPECT_CAP(bigint, isN, nD, cap)     \
    QTEST_EXPECT(bigint != NULL);                    \
    QTEST_EXPECT_EQUALS(isN, (bigint)->isNegative);  \
    QTEST_EXPECT_EQUALS(nD, (bigint)->nDigits);      \
    QTEST_EXPECT((bigint)->capacity >= cap);         \
    QTEST_EXPECT((bigint)->digits != NULL);

// we dont have to pass expected capacity, since we always
// expect the capacity to be at least as big as the number of digits
#define BIG_INT_EXPECT(bigint, isN, nD)      \
    BIG_INT_EXPECT_CAP(bigint, isN, nD, nD)

#define INIT_LIST(...) __VA_ARGS__
#define JOIN_(x, y) x##y
#define JOIN(x, y) JOIN_(x, y)
#define VARNAME(Var) JOIN(Var, __LINE__)
#define BIG_INT_EXPECT_DIGITS(bigint, eDigits)        \
    digit_t JOIN(d, __LINE__)[] = INIT_LIST eDigits;  \
    for (size_t i = 0; i < (bigint)->nDigits; i++) {  \
        QTEST_EXPECT_EQUALS(JOIN(d, __LINE__)[i],     \
                            ((bigint)->digits)[i]);   \
    }

#endif  // TEST_UTILS_H_