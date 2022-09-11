#ifndef AES2_H_
#define AES2_H_

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>  // bzero

#include <stdio.h>

#include "common.h"
#include "tables.h"

// number of 32 bit words in the state == 16 bytes
#define Nb 4
// number of 32 bit words in the key: one of 4, 6, 8 == 16, 24, 32 bytes
// #ifndef Nk
// #define Nk 4
// #endif
// number of rounds (func of Nk and Nb (Nb is fixed)): one of 10, 12, 14
#define GET_Nr(x) (x + 6)
#define NK_BYTES_TO_WORDS(x) (x / 4)

#define STATE_NROWS 4
#define STATE_NCOLS Nb
#define STATE_SIZE (STATE_NROWS * STATE_NCOLS)

// irriducible polynomial in GF(2^8)
#define IPOLY 0X1b

// byte at row r, column c of state array s
// #define SLOC(s, r, c) (s)[r * STATE_NROWS + c]
#define SLOC(s, r, c) (s)[r + c * STATE_NCOLS]
// bit at location i of a byte b
#define BLOC(b, i) (b & (1 << i)) >> i
// multiplication of an element of GF(2^8) (represented as a
// byte b) with 2
#define MULT_BY_2(b)  ((BLOC(b, 7)) ? (b << 1) ^ IPOLY : b << 1)
#define MULT_BY_3(b)  (MULT_BY_2(b) ^ b)
#define MULT_BY_9(b)  (mBy9[b])
#define MULT_BY_11(b) (mBy11[b])
#define MULT_BY_13(b) (mBy13[b])
#define MULT_BY_14(b) (mBy14[b])
// #define MULT_BY_9(b)  (MULT_BY_2(MULT_BY_2(MULT_BY_2(b))) ^ b)
// #define MULT_BY_11(b) (MULT_BY_2((MULT_BY_2(MULT_BY_2(b)) ^ b)) ^ b)
// #define MULT_BY_13(b) (MULT_BY_2(MULT_BY_2(MULT_BY_2(b) ^ b)) ^ b)
// #define MULT_BY_14(b) (MULT_BY_2(MULT_BY_2(MULT_BY_2(b) ^ b) ^ b))

void AES_GenerateKeySchedule(byte key[], size_t Nk, byte schedule[]);
void AES_SubBytes(byte bytes[], size_t nBytes);
void AES_InvSubBytes(byte bytes[], size_t nBytes);
void AES_RotBytes(byte bytes[], size_t nBytes);
// pre: both arrays have the same size, nBytes
// post: xor of both arrays byte-wise is stored in bytesA
void AES_XORBytes(byte bytesA[], byte bytesB[], size_t nBytes);
// retrieves the rcon value for i and puts all 4 bytes in the given result array
void AES_Rcon_i(size_t i, byte result[]);

// main algorithm functions
void AES_Encipher(byte input[], byte key[], size_t Nk, byte output[]);
void AES_ShiftRows(byte state[]);
void AES_MixColumns(byte state[]);
void AES_AddRoundKey(byte state[], byte roundKey[]);

void AES_Decipher(byte input[], byte key[], size_t Nk, byte output[]);
void AES_InvShiftRows(byte state[]);
void AES_InvMixColumns(byte state[]);

// helper functions:
// byte AES_MultiplyBytes(byte a, byte b);
// byte AES_AddBytes(byte a, byte b);

// debugging functions:
void PrintBytes(byte bytes[], size_t nBytes) {
    for (size_t i = 0; i < nBytes; i++) {
        printf("%02x ", bytes[i]);
    }
}
void PrintBytesNL(byte bytes[], size_t nBytes) {
    PrintBytes(bytes, nBytes);
    printf("\n");
}

void AES_GenerateKeySchedule(byte key[], size_t Nk, byte schedule[]) {
    // generates Nb(Nr + 1) words == Nb(Nr + 1) * 4 bytes
    // each of the Nr rounds requires Nb words of key data,
    // plus an initial Nb words

    // first Nk words == Nk * 4 bytes are copied from the key
    for (size_t i = 0; i < 4 * Nk; i++) {
        schedule[i] = key[i];
    }

    for (size_t i = 4 * Nk; i < 4 * Nb * (GET_Nr(Nk) + 1); i += 4) {
        // store the last word (as an array of 4 bytes)
        byte temp[4] = {schedule[i - 4], schedule[i - 3], schedule[i - 2], schedule[i - 1]};

        // printf("%zu: ", i / 4);
        // PrintBytesNL(temp, 4);

        if ((i / 4) % Nk == 0) {
            AES_RotBytes(temp, 4);
            AES_SubBytes(temp, 4);
            byte rcon_i[4];
            AES_Rcon_i((i / 4) / Nk, rcon_i);
            AES_XORBytes(temp, rcon_i, 4);
        } else if (Nk > 6 && (i / 4) % Nk == 4) {
            AES_SubBytes(temp, 4);
        }

        byte temp2[4] = {schedule[i - (Nk * 4)], schedule[i - (Nk * 4) + 1], schedule[i - (Nk * 4) + 2], schedule[i - (Nk * 4) + 3]};
        AES_XORBytes(temp2, temp, 4);
        for (size_t j = 0; j < 4; j++) {
            schedule[i + j] = temp2[j];
        }
    }
}

void AES_SubBytes(byte bytes[], size_t nBytes) {
    for (size_t i = 0; i < nBytes; i++) {
        bytes[i] = sbox[bytes[i]];
    }
}

void AES_RotBytes(byte bytes[], size_t nBytes) {
    // TODO: check nBytes >= 1 and return immediately otherwise
    // TODO: consider adding a parameter to indicate the number
    // of places to shift by; currently it shifts by exactly one
    // to the left with wrap-around
    byte t0 = bytes[0];
    for (size_t i = 0; i < nBytes - 1; i++) {
        bytes[i] = bytes[i + 1];
    }
    bytes[nBytes - 1] = t0;
}

void AES_XORBytes(byte bytesA[], byte bytesB[], size_t nBytes) {
    for (size_t i = 0; i < nBytes; i++) {
        bytesA[i] ^= bytesB[i];
    }
}

void AES_Rcon_i(size_t i, byte result[]) {
    // we observe that the first byte in each
    // lookup is the only non-zero element in the result byte array.
    // so, we only store this non-zero value in the rcon array
    // and set the remaining elements of the result array to 0.
    // see the wikipedia page for the table
    result[0] = Rcon[i];
    bzero(result + 1, 3);
}

void AES_Encipher(byte input[], byte key[], size_t Nk, byte output[]) {
    byte state[STATE_SIZE];
    // copy input into the state
    memcpy(state, input, STATE_SIZE);

    // create key schedule
    byte schedule[4 * Nb * (GET_Nr(Nk) + 1)];
    AES_GenerateKeySchedule(key, Nk, schedule);

    AES_AddRoundKey(state, schedule);

    for (size_t i = 1; i < GET_Nr(Nk); i++) {
        AES_SubBytes(state, STATE_NROWS * STATE_NCOLS);
        AES_ShiftRows(state);
        AES_MixColumns(state);
        AES_AddRoundKey(state, &(schedule[STATE_NROWS * STATE_NCOLS * i]));
    }
    AES_SubBytes(state, STATE_NROWS * STATE_NCOLS);
    AES_ShiftRows(state);
    AES_AddRoundKey(state, &(schedule[STATE_NROWS * STATE_NCOLS * GET_Nr(Nk)]));

    // copy state to output
    memcpy(output, state, STATE_NROWS * STATE_NCOLS);
}

void AES_ShiftRows(byte state[]) {
    for (size_t r = 1; r < STATE_NROWS; r++) {
        for (size_t i = 0; i < r; i++) {
            byte temp[STATE_NCOLS];
            for (size_t j = 0; j < STATE_NCOLS; j++) {
                temp[j] = SLOC(state, r, j);
            }
            AES_RotBytes(temp, STATE_NCOLS);
            for (size_t j = 0; j < STATE_NCOLS; j++) {
                SLOC(state, r, j) = temp[j];
            }
            // AES_RotBytes(state + STATE_NROWS * r, STATE_NCOLS);
        }
    }
}

void AES_MixColumns(byte state[]) {
    // since we need the original state values for every
    // transformed value, we need to store results in a temp
    // array and copy them to the output array at the end.
    // we need one entry for each row element, since each column
    // is evaluated independently
    for (size_t c = 0; c < STATE_NCOLS; c++) {
        byte result[STATE_NROWS];
        result[0] = MULT_BY_2(SLOC(state, 0, c)) ^
                    MULT_BY_3(SLOC(state, 1, c)) ^
                    SLOC(state, 2, c) ^
                    SLOC(state, 3, c);

        result[1] = SLOC(state, 0, c) ^
                    MULT_BY_2(SLOC(state, 1, c)) ^
                    MULT_BY_3(SLOC(state, 2, c)) ^
                    SLOC(state, 3, c);

        result[2] = SLOC(state, 0, c) ^
                    SLOC(state, 1, c) ^
                    MULT_BY_2(SLOC(state, 2, c)) ^
                    MULT_BY_3(SLOC(state, 3, c));

        result[3] = MULT_BY_3(SLOC(state, 0, c)) ^
                    SLOC(state, 1, c) ^
                    SLOC(state, 2, c) ^
                    MULT_BY_2(SLOC(state, 3, c));

        // copy the computed values to the output array
        for (size_t r = 0; r < STATE_NROWS; r++) {
            SLOC(state, r, c) = result[r];
        }
    }
}

void AES_AddRoundKey(byte state[], byte roundKey[]) {
    AES_XORBytes(state, roundKey, STATE_NROWS * STATE_NCOLS);
}

// inverse functions

void AES_Decipher(byte input[], byte key[], size_t Nk, byte output[]) {
    byte state[STATE_NROWS * STATE_NCOLS];
    // copy input into the state
    memcpy(state, input, STATE_NROWS * STATE_NCOLS);

    // create key schedule
    byte schedule[4 * Nb * (GET_Nr(Nk) + 1)];
    AES_GenerateKeySchedule(key, Nk, schedule);

    AES_AddRoundKey(state, &(schedule[STATE_NROWS * STATE_NCOLS * GET_Nr(Nk)]));


    for (size_t i = GET_Nr(Nk) - 1; i != 0; i--) {
        AES_InvShiftRows(state);
        AES_InvSubBytes(state, STATE_NROWS * STATE_NCOLS);
        AES_AddRoundKey(state, &(schedule[STATE_NROWS * STATE_NCOLS * i]));
        AES_InvMixColumns(state);
    }

    AES_InvShiftRows(state);
    AES_InvSubBytes(state, STATE_NROWS * STATE_NCOLS);
    AES_AddRoundKey(state, schedule);

    // copy state to output
    memcpy(output, state, STATE_NROWS * STATE_NCOLS);
}

void AES_InvShiftRows(byte state[]) {
    // TODO: optimize
    for (size_t r = 1; r < STATE_NROWS; r++) {
        for (size_t i = 0; i < 4 - r; i++) {
            byte temp[STATE_NCOLS];
            for (size_t j = 0; j < STATE_NCOLS; j++) {
                temp[j] = SLOC(state, r, j);
            }
            AES_RotBytes(temp, STATE_NCOLS);
            for (size_t j = 0; j < STATE_NCOLS; j++) {
                SLOC(state, r, j) = temp[j];
            }
            // AES_RotBytes(state + STATE_NROWS * r, STATE_NCOLS);
        }
    }
}

void AES_InvMixColumns(byte state[]) {
    for (size_t c = 0; c < STATE_NCOLS; c++) {
        byte result[STATE_NROWS];
        result[0] = MULT_BY_14(SLOC(state, 0, c)) ^
                    MULT_BY_11(SLOC(state, 1, c)) ^
                    MULT_BY_13(SLOC(state, 2, c)) ^
                    MULT_BY_9(SLOC(state, 3, c));

        result[1] = MULT_BY_9(SLOC(state, 0, c))  ^
                    MULT_BY_14(SLOC(state, 1, c)) ^
                    MULT_BY_11(SLOC(state, 2, c)) ^
                    MULT_BY_13(SLOC(state, 3, c));

        result[2] = MULT_BY_13(SLOC(state, 0, c)) ^
                    MULT_BY_9(SLOC(state, 1, c))  ^
                    MULT_BY_14(SLOC(state, 2, c)) ^
                    MULT_BY_11(SLOC(state, 3, c));

        result[3] = MULT_BY_11(SLOC(state, 0, c)) ^
                    MULT_BY_13(SLOC(state, 1, c)) ^
                    MULT_BY_9(SLOC(state, 2, c))  ^
                    MULT_BY_14(SLOC(state, 3, c));

        // copy the computed values to the output array
        // TODO: can we just copy the array sequentially
        // with the correct column offset instead of
        // using SLOC ? since the state is column-major order ?
        for (size_t r = 0; r < STATE_NROWS; r++) {
            SLOC(state, r, c) = result[r];
        }
    }
}

void AES_InvSubBytes(byte bytes[], size_t nBytes) {
    for (size_t i = 0; i < nBytes; i++) {
        bytes[i] = invSbox[bytes[i]];
    }
}

#endif  // AES2_H_