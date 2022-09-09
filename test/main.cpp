extern "C" {
    #include "../include/aes.h"
    // #include "../include/bigint.h"
}

#include "qtest.hpp"

// bigint tests:
// #include "test_utils.hpp"
// #include "test_io_internals.hpp"
// #include "test_add2.hpp"
// #include "test_subtract2.hpp"
// #include "test_multiply.hpp"

// aes tests:
#include "test_aes.hpp"

int main(int argc, char const *argv[]) {
    QTEST_RUN_ALL();
    return 0;
}
