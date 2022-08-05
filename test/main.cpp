extern "C" {
    #include "../include/crypt.h"
}

#include "qtest.hpp"

QTEST_CASE(BigInt, Add) {
    QTEST_EXPECT(hello() == 69);
}

int main(int argc, char const *argv[]) {
    QTEST_RUN_ALL();
    return 0;
}
