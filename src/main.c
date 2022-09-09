#include <stdio.h>

#include "../include/crypt.h"

int main(int argc, char const *argv[]) {

    Crypt_EncipherRange("./src/file.txt", "./src/cipher.txt", 0, 1000);
    Crypt_DecipherRange("./src/cipher.txt", "./src/decrypted.txt", 0, 1000);

    return 0;
}
