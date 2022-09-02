#ifndef AES_H_
#define AES_H_

#include <stdint.h>
#include <stdlib.h>

// key length; number of bits; 128, 192, 256
// #define K 128
// key length; number of 32-bit words; 4, 6, 8
#define Nk 4
// number of rounds
#define Nr (10 + (Nk - 4))
// block size (always 4) (in bytes) (number of columns in the state)
#define Nb 4
// number of rows in state
#define NROWS 4

// locate row, column in a state object
#define SLOC(a, r, c) ((a)->data[((r) * (a)->nCols) + (c)])
// locate row, column in a key schedule object
// #define KSLOC(a, r, c) ((a)->data[((r) * (a)->nCols) + (c)])
// locate row, column in a key object
#define KLOC(a, r, c) ((a)->data[((r) * (a)->nCols) + (c)])

// most and least significant nibble of a byte x
#define MSN(x) ((x >> 4) & 0x80)
#define LSN(x) (x & 0x08)

typedef uint8_t  Byte;
typedef uint32_t Word;

typedef struct {
    size_t nRows;
    size_t nCols;
    Byte* data;
} State;

typedef struct {
    size_t nWords;
    Word* data;
} Key;

typedef struct {
    size_t nKeys;
    Key* data;
} KeySchedule;

// source: https://en.wikipedia.org/wiki/Rijndael_S-box
#define ROTL8(x,shift) ((Byte) ((x) << (shift)) | ((x) >> (8 - (shift))))
void SBoxInit(Byte sbox[256]) {
	Byte p = 1, q = 1;
	/* loop invariant: p * q == 1 in the Galois field */
	do {
		/* multiply p by 3 */
		p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

		/* divide q by 3 (equals multiplication by 0xf6) */
		q ^= q << 1;
		q ^= q << 2;
		q ^= q << 4;
		q ^= q & 0x80 ? 0x09 : 0;

		/* compute the affine transformation */
		Byte xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

		sbox[p] = xformed ^ 0x63;
	} while (p != 1);

	/* 0 is a special case since it has no inverse */
	sbox[0] = 0x63;
}

// main interface function for enciphering data
void AES_Encipher(const State* input, State* output, KeySchedule* keys);
void AES_Decipher(const State* input, State* output, KeySchedule* keys);

// internal helper functions
void AddRoundKey(State* state, Key* key);
void    SubBytes(State* state);
void   ShiftRows(State* state);
void  MixColumns(State* state);
Word     RotWord(Word word);
Word     SubWord(Word word);
void   ExpandKey(Key* key, KeySchedule* schedule);

void State_CopyTo(const State* src, State* dest);

State* State_New(size_t nRows, size_t nCols) {
    State* state = (State*) malloc(sizeof(State));
    if (!state) {
        return NULL;
    }

    state->data = (Byte*) malloc((nRows * nCols) * sizeof(Byte));
    if (!(state->data)) {
        free(state);
        return NULL;
    }

    state->nCols = nCols;
    state->nRows = nRows;
    return state;
}

void State_Resize(State* state, size_t nRows, size_t nCols) {
    state->data = (Byte*) realloc(state->data, nRows * nCols * sizeof(Byte));
    // TODO: error handling
    state->nRows = nRows;
    state->nCols = nCols;
}

State* State_Copy(const State* src) {
    State* state = State_New(src->nRows, src->nCols);
    if (!state) {
        return NULL;
    }
    State_CopyTo(src, state);
    return state;
}

void State_CopyTo(const State* src, State* dest) {
    State_Resize(dest, src->nRows, src->nCols);
    dest->nRows = src->nRows;
    dest->nCols = src->nCols;
    // TODO: use memcpy instead
    for (size_t i = 0; i < src->nCols * src->nRows; i++) {
        dest->data[i] = src->data[i];
    }
}

// create the sbox
static Byte sbox[256];

void AES_Encipher(const State* input, State* output, KeySchedule* keys) {
    // keys shall contain Nr + 1 keys, each of which contains
    // Nb words

    SBoxInit(sbox);

    // copy the input state into the working state
    State* state = State_Copy(input);

    AddRoundKey(state, &((keys->data)[0]));

    for (size_t i = 1; i < keys->nKeys - 2; i++) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, &((keys->data)[i]));
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, &((keys->data)[keys->nKeys - 1]));

    State_CopyTo(state, output);
}

void AddRoundKey(State* state, Key* key) {

}
void SubBytes(State* state) {
    printf("%d\n", sbox[0x00]);
}
void ShiftRows(State* state) {

}
void MixColumns(State* state) {

}

Word SubWord(Word word) {
    Byte i0 = (word >> (sizeof(Byte) * 0)) & 0xFF;
    Byte i1 = (word >> (sizeof(Byte) * 1)) & 0xFF;
    Byte i2 = (word >> (sizeof(Byte) * 2)) & 0xFF;
    Byte i3 = (word >> (sizeof(Byte) * 3)) & 0xFF;

    Byte r0 = sbox[i0];
    Byte r1 = sbox[i1];
    Byte r2 = sbox[i2];
    Byte r3 = sbox[i3];

    Word ret = r0;

    return ret;
}

Word RotWord(Word word) {

}

void ExpandKey(Key* key, KeySchedule* schedule) {
    // precondition: key->nWords == Nb, since each round "requires Nb words of key data"
    // generates a total of Nb(Nr + 1) words; or Nr + 1 keys
    // initialized in a KeySchedule struct
    schedule->nKeys = Nr + 1;
}

#endif  // AES_H_