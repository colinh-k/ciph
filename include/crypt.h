#ifndef CC_CRYPT_H_
#define CC_CRYPT_H_

#include <stdlib.h>  // for arc4random

// #include "bigint.h"
#include "aes.h"

#define MIN(a, b) ((a < b) ? a : b)
// indicates the last byte of the file
#define CRYPT_EOF ((size_t) -1)
// indicates the first byte of the file
#define CRYPT_SOF (0)
#define CRYPT_NOFILE ("")
// max key size in bytes
#define CRYPT_MAX_KEY_SIZE 32
#define CRYPT_CP_BUF_SIZE 1024

// calculate the last enciphered byte from a range of plaintext bytes.
// essentially, deciphering requires a range that is a multiple of STATE_SIZE,
// so it can correctly remove padding. if a range of plaintext bytes
// is a multiple of STATE_SIZE, an extra STATE_SIZE bytes will be added
// to the ciphertext as padding so deciphering can unambiguously remove
// padding
#define CRYPT_CALC_ENDPT(b, e) (e + (STATE_SIZE - ((e - b) % STATE_SIZE)))

typedef void (*Crypt_AESFn)(byte input[], byte key[], size_t Nk, byte output[]);

void Crypt_EncipherRange(const char* fnameIn,
                         const char* fnameKey,
                         const char* fnameOut,
                         size_t firstByte,
                         size_t lastByte);

void Crypt_DecipherRange(const char* fnameIn,
                         const char* fnameKey,
                         const char* fnameOut,
                         size_t firstByte,
                         size_t lastByte);

void Crypt_Transform(FILE* fileIn, FILE* fileOut, size_t nBlocks, Crypt_AESFn aesfn, byte key[], size_t keySize);
// copies nBytes bytes from fileIn to fileOut
void Crypt_CopyFile(FILE* fileIn, FILE* fileOut, size_t nBytes);

// returns number of bytes in the file (i.e., the size of the key)
off_t Crypt_KeyFromFile(const char* fname, byte key[]);
void Crypt_GenerateKeyFile(const char* fname, size_t keySize);

void Crypt_EncipherRange(const char* fnameIn,
                         const char* fnameKey,
                         const char* fnameOut,
                         size_t firstByte,
                         size_t lastByte) {
    FILE* fileIn = fopen(fnameIn, "rb");

    // get the size of the file
    fseek(fileIn, 0L, SEEK_END);
    off_t fsize = ftell(fileIn);
    // rewind to the first byte
    // fseek(fileIn, firstByte, SEEK_SET);
    rewind(fileIn);
    // if the given lastByte is out of range, just encrypt to the end of the file
    lastByte = MIN(lastByte, fsize);

    byte key[CRYPT_MAX_KEY_SIZE];
    off_t keySize = Crypt_KeyFromFile(fnameKey, key);

    FILE* fileOut = fopen(fnameOut, "wb");
    size_t nBlocks = (lastByte - firstByte) / STATE_SIZE;
    // number of pad bytes required to align last block to a length STATE_SIZE bytes
    // if the range is a multiple of STATE_SIZE, we pad an extra STATE_SIZE bytes
    // to the end, each with value STATE_SIZE
    byte nPad = STATE_SIZE - ((lastByte - firstByte) % STATE_SIZE);

    Crypt_CopyFile(fileIn, fileOut, firstByte);

    // encipher the range of bytes (except the last one which will be padded below)
    Crypt_Transform(fileIn, fileOut, nBlocks, AES_Encipher, key, keySize);

    // pad the final block
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

    // copy any remaining bytes after the range
    Crypt_CopyFile(fileIn, fileOut, fsize - lastByte);

    fclose(fileIn);
    fclose(fileOut);
}

void Crypt_DecipherRange(const char* fnameIn,
                         const char* fnameKey,
                         const char* fnameOut,
                         size_t firstByte,
                         size_t lastByte) {
    FILE* fileIn = fopen(fnameIn, "rb");

    // get the size of the file
    fseek(fileIn, 0L, SEEK_END);
    size_t fsize = ftell(fileIn);
    // fseek(fileIn, firstByte, SEEK_SET);
    rewind(fileIn);
    // if the given lastByte is out of range, just encrypt to the end of the file
    lastByte = MIN(lastByte, fsize);

    // PRE: we assert the range MUST be a multiple of STATE_SIZE
    if ((lastByte - firstByte) % STATE_SIZE != 0) {
        // not a multiple of the state_size
        fprintf(stderr, "Decipher error: the range %zu to %zu is not" \
                         "a multiple of the block size %d.\n",
                         firstByte, lastByte, STATE_SIZE);
        fclose(fileIn);
        return;
    }

    byte key[CRYPT_MAX_KEY_SIZE];
    off_t keySize = Crypt_KeyFromFile(fnameKey, key);

    FILE* fileOut = fopen(fnameOut, "wb");
    size_t nBlocks = (lastByte - firstByte) / STATE_SIZE;

    // copy the bytes before the range
    Crypt_CopyFile(fileIn, fileOut, firstByte);

    // decipher the range (the final block with padding will be delt with below)
    // check for nBlocks > 1, since we subtract 1 from nBlocks.
    // this could cause underflow if nBlocks == 1 or 0
    for (size_t i = 0; nBlocks > 1 && i < nBlocks - 1; i++) {
        byte ciphertext[STATE_SIZE];
        byte  decrypted[STATE_SIZE];

        fread(ciphertext, sizeof(byte), STATE_SIZE, fileIn);
        AES_Decipher(ciphertext, key, NK_BYTES_TO_WORDS(keySize), decrypted);
        fwrite(decrypted, sizeof(byte), STATE_SIZE, fileOut);
    }

    // decipher the last block (by removing padding)
    byte ciphertext[STATE_SIZE];
    byte  decrypted[STATE_SIZE];

    fread(ciphertext, sizeof(byte), STATE_SIZE, fileIn);
    AES_Decipher(ciphertext, key, 4, decrypted);

    // remove the padding present in the final state_size bytes of the file
    // there will by padByte bytes with value padByte
    byte padByte = decrypted[STATE_SIZE - 1];
    if (padByte > STATE_SIZE) {
        fprintf(stderr, "Decipher error: encountered malformed padding" \
                        " sequence. Encountered a padding sequence of"  \
                        " size %d that exceeds the block size %d.\n",
                        padByte, STATE_SIZE);
        fclose(fileIn);
        fclose(fileOut);
        return;
    }

    // write the non-padding bytes to the output
    fwrite(decrypted, sizeof(byte), STATE_SIZE - padByte, fileOut);

    // copy the remaining bytes after the range
    Crypt_CopyFile(fileIn, fileOut, fsize - lastByte);

    fclose(fileIn);
    fclose(fileOut);
}

off_t Crypt_KeyFromFile(const char* fname, byte key[]) {
    FILE* fileKey = fopen(fname, "rb");
    // get the size of the key file
    fseek(fileKey, 0L, SEEK_END);
    off_t fsize = ftell(fileKey);
    rewind(fileKey);
    // read the entire key from the file
    fread(key, sizeof(byte), fsize, fileKey);
    fclose(fileKey);
    return fsize;
}

void Crypt_GenerateKeyFile(const char* fname, size_t keySize) {
    FILE* fileKey = fopen(fname, "wb");

    byte key[CRYPT_MAX_KEY_SIZE];
    arc4random_buf(key, keySize);
    fwrite(key, sizeof(byte), keySize, fileKey);

    fclose(fileKey);
}

void Crypt_Transform(FILE* fileIn, FILE* fileOut, size_t nBlocks, Crypt_AESFn aesfn, byte key[], size_t keySize) {
    byte  plaintext[STATE_SIZE];
    byte ciphertext[STATE_SIZE];

    for (size_t i = 0; i < nBlocks; i++) {
        fread(plaintext, sizeof(byte), STATE_SIZE, fileIn);
        aesfn(plaintext, key, NK_BYTES_TO_WORDS(keySize), ciphertext);
        fwrite(ciphertext, sizeof(byte), STATE_SIZE, fileOut);
    }
}

void Crypt_CopyFile(FILE* fileIn, FILE* fileOut, size_t nBytes) {
    // assume: fileIn contains at least nBytes
    byte buf[CRYPT_CP_BUF_SIZE];
    size_t nCopiedBytes = 0;
    size_t nRounds = nBytes / CRYPT_CP_BUF_SIZE;
    for (size_t i = 0; i < nRounds; i++) {
        size_t nReadBytes = fread(buf, sizeof(byte), CRYPT_CP_BUF_SIZE, fileIn);
        fwrite(buf, sizeof(byte), nReadBytes, fileOut);
        nCopiedBytes += nReadBytes;
    }
    size_t nRemainingBytes = nBytes - nCopiedBytes;
    size_t nReadBytes = fread(buf, sizeof(byte), nRemainingBytes, fileIn);
    fwrite(buf, sizeof(byte), nReadBytes, fileOut);
}

#endif  // CC_CRYPT_H_