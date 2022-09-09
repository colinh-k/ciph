#ifndef CC_CRYPT_H_
#define CC_CRYPT_H_

// #include "bigint.h"
#include "aes.h"

#define MIN(a, b) ((a < b) ? a : b)
// indicates the last byte of the file
#define CRYPT_EOF ((size_t) -1)
// indicates the first byte of the file
#define CRYPT_SOF (0)

void Crypt_EncipherRange(const char* fnameIn, const char* fnameOut, size_t firstByte, size_t lastByte);
void Crypt_DecipherRange(const char* fnameIn, const char* fnameOut, size_t firstByte, size_t lastByte);

void Crypt_EncipherRange(const char* fnameIn, const char* fnameOut, size_t firstByte, size_t lastByte) {
    FILE* fileIn = fopen(fnameIn, "rb");

    // get the size of the file
    fseek(fileIn, 0L, SEEK_END);
    off_t fsize = ftell(fileIn);
    // rewind to the first byte
    fseek(fileIn, firstByte, SEEK_SET);
    // if the given lastByte is out of range, just encrypt to the end of the file
    lastByte = MIN(lastByte, fsize);

    byte key[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    FILE* fileOut = fopen(fnameOut, "wb");
    size_t nBlocks = (lastByte - firstByte) / STATE_SIZE;
    // number of pad bytes required
    byte nPad = STATE_SIZE - ((lastByte - firstByte) % STATE_SIZE);

    for (size_t i = 0; i < nBlocks; i++) {
        byte  plaintext[STATE_SIZE];
        byte ciphertext[STATE_SIZE];

        fread(plaintext, sizeof(byte), STATE_SIZE, fileIn);
        AES_Encipher(plaintext, key, 4, ciphertext);
        fwrite(ciphertext, sizeof(byte), STATE_SIZE, fileOut);
    }

    byte  plaintext[STATE_SIZE];
    byte ciphertext[STATE_SIZE];

    // pad the final block
    // NOTE: if STATE_SIZE - nPad == 0, then fread returns 0 and does nothing (see fread spec)
    fread(plaintext, sizeof(byte), STATE_SIZE - nPad, fileIn);
    // pad the remaining bytes
    for (size_t i = STATE_SIZE - nPad; i < STATE_SIZE; i++) {
        plaintext[i] = nPad;
    }

    AES_Encipher(plaintext, key, 4, ciphertext);
    fwrite(ciphertext, sizeof(byte), STATE_SIZE, fileOut);

    fclose(fileIn);
    fclose(fileOut);
}

void Crypt_DecipherRange(const char* fnameIn, const char* fnameOut, size_t firstByte, size_t lastByte) {
    FILE* fileIn = fopen(fnameIn, "rb");

    // get the size of the file
    fseek(fileIn, 0L, SEEK_END);
    size_t fsize = ftell(fileIn);
    fseek(fileIn, firstByte, SEEK_SET);
    // if the given lastByte is out of range, just encrypt to the end of the file
    lastByte = MIN(lastByte, fsize);

    // PRE: we assert the range MUST be a multiple of STATE_SIZE
    // printf("%zu\n", (lastByte - firstByte) % STATE_SIZE);
    if ((lastByte - firstByte) % STATE_SIZE != 0) {
        // not a multiple of the state_size
        fprintf(stderr, "Decipher error: the range %zu to %zu is not a multiple of the block size %d.\n", firstByte, lastByte, STATE_SIZE);
        fclose(fileIn);
        return;
    }

    byte key[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    FILE* fileOut = fopen(fnameOut, "wb");
    size_t nBlocks = (lastByte - firstByte) / STATE_SIZE;

    // check for nBlocks > 0, since we subtract 1 from nBlocks.
    // this could cause underflow if nBlocks == 1 or 0
    for (size_t i = 0; nBlocks > 1 && i < nBlocks - 1; i++) {
        byte ciphertext[STATE_SIZE];
        byte  decrypted[STATE_SIZE];

        fread(ciphertext, sizeof(byte), STATE_SIZE, fileIn);
        AES_Decipher(ciphertext, key, 4, decrypted);
        fwrite(decrypted, sizeof(byte), STATE_SIZE, fileOut);
    }


    byte ciphertext[STATE_SIZE];
    byte  decrypted[STATE_SIZE];

    fread(ciphertext, sizeof(byte), STATE_SIZE, fileIn);
    AES_Decipher(ciphertext, key, 4, decrypted);

    // remove the padding present in the final state_size bytes of the file
    // there will by padByte bytes with value padByte
    byte padByte = decrypted[STATE_SIZE - 1];
    if (padByte > STATE_SIZE) {
        fprintf(stderr, "Decipher error: encountered malformed padding sequence. \
                         Encountered a padding sequence %d that exceeds the \
                         block size %d.\n",
                         padByte, STATE_SIZE);
        fclose(fileIn);
        fclose(fileOut);
        return;
    }
    // for (size_t i = 0; i < padByte; i++) {
    //     if (decrypted[STATE_SIZE - i - 1] != padByte) {
    //         fprintf(ferror, "Decipher error: encountered malformed padding sequence. \
    //                          Expected %d bytes of %02x, but encountered %02x instead.\n",
    //                          padByte, padByte, decrypted[i]);
    //         fclose(fileIn);
    //         fclose(fileOut);
    //         return;
    //     }
    // }

    fwrite(decrypted, sizeof(byte), STATE_SIZE - padByte, fileOut);

    fclose(fileIn);
    fclose(fileOut);
}

#endif  // CC_CRYPT_H_