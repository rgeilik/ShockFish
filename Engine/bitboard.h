#pragma once
#include "utility.h"
#include <stdio.h>
#include <inttypes.h>

#define USE_32_BIT_MULTIPLICATIONS

typedef unsigned long long BBOARD; // typedef for bitboard type

typedef struct {
	BBOARD  state;
} prng_state;

BBOARD pawn_attacks[2][64]; // Declare pawn attack table
BBOARD knight_attacks[64];   // Declare knight attack tables
BBOARD king_attacks[64];     // Declare king attack tables

BBOARD pawn_attack_mask(int color, int square);
BBOARD knight_attack_mask(int square);
BBOARD king_attack_mask(int square);
void init_leaper_attacks(); // Function to initialize leaper attacks
void printBitboard(BBOARD bitboard); // Declare print function if you need to use it in test.c

// Number of relevant occupancy bits for each square for the Rook
const int rook_relevant_bitCount[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

// Number of relevant occupancy bits for each square for the Bishop
const int bishop_relevant_bitCount[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

static inline BBOARD prng_u64(prng_state* const p)
{
	BBOARD  state = p->state;
	state ^= state >> 12;
	state ^= state << 25;
	state ^= state >> 27;
	p->state = state;
	return state * UINT64_C(2685821657736338717);
}

// Function to initialize the PRNG
void prng_init(prng_state* p) {
	p->state = (BBOARD)time(NULL);
}

/* int countNonZeroElements(int arr[], int size) {
	int count = 0;

	for (int i = 0; i < size; i++) {
		if (arr[i] != 0) { count++; }
	}
	return count;
} */


