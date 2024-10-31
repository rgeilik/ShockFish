#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "bitboard.h"


/* --------------LEAPER PIECES ATTACKS----------------- */

extern BBOARD pawn_attacks[2][BOARD_SIZE]; // Define pawn attack table
extern BBOARD knight_attacks[BOARD_SIZE]; // Define knight attack tables
extern BBOARD king_attacks[BOARD_SIZE]; // Define king attack tables



// Function to count set bits in bitboard
static inline int count_bits(BBOARD bitboard) {
	int count = 0;

	while (bitboard) {
		count++;
		bitboard &= bitboard - 1;
	}

	return count;
}

/* static int NumberOfSetBits(unsigned long long i)
{
	i = i - ((i >> 1) & 0x5555555555555555UL);
	i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
	return (int)(((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL >> 56);
} */

//GENERATE PAWN ATTACK MASK
BBOARD pawn_attack_mask(int color, int square) {
	BBOARD attacks = 0ULL; // Pawn attack mask bitboard
	
	BBOARD pawn = 0ULL; // Pawn position bitboard
	set_bit(pawn, square); // Set bit at pawn position

	if (color) {
		if (pawn >> 7 & not_A_file) { attacks |= (pawn >> 7); }
		if (pawn >> 9 & not_H_file) { attacks |= (pawn >> 9); }
	
	}

	else {
		if (pawn << 7 & not_H_file) { attacks |= (pawn << 7); }
		if (pawn << 9 & not_A_file) { attacks |= (pawn << 9); }

	}
	

	return attacks;

}

//GENERATE KNIGHT ATTACK MASK
BBOARD knight_attack_mask(int square) {
	BBOARD attacks = 0ULL;

	BBOARD knight = 0ULL;
	set_bit(knight, square);

		if (knight >> 17 & not_H_file) { attacks |= (knight >> 17); }
		if (knight >> 15 & not_A_file) { attacks |= (knight >> 15); }
		if (knight >> 10 & not_HG_file) { attacks |= (knight >> 10); }
		if (knight >> 6 & not_AB_file) { attacks |= (knight >> 6); }

		if (knight << 17 & not_A_file) { attacks |= (knight << 17); }
		if (knight << 15 & not_H_file) { attacks |= (knight << 15); }
		if (knight << 10 & not_AB_file) { attacks |= (knight << 10); }
		if (knight << 6 & not_HG_file) { attacks |= (knight << 6); }

		return attacks;


}

//GENERATE KING ATTACK MASK
BBOARD king_attack_mask(int square) {
	BBOARD attacks = 0ULL;

	BBOARD king = 0ULL;
	set_bit(king, square);

	if (king << 9 & not_A_file) { attacks |= (king << 9); }
	attacks |= (king << 8);
	if (king << 7 & not_H_file) { attacks |= (king << 7); }
	if (king << 1 & not_A_file) { attacks |= (king << 1); }

	if (king >> 9 & not_H_file) { attacks |= (king >> 9); }
	attacks |= (king >> 8);
	if (king >> 7 & not_A_file) { attacks |= (king >> 7); }
	if (king >> 1 & not_H_file) { attacks |= (king >> 1); }
	
	return attacks;
}

// INITIALISE ATTACKS FOR "LEAPER" PIECES (PAWNS, KNIGHTS AND KING)
void init_leaper_attacks() {
	
	for (int square = 0; square < BOARD_SIZE; square++) {
		//Init pawn attacks table
		pawn_attacks[white][square] = pawn_attack_mask(white, square);
		pawn_attacks[black][square] = pawn_attack_mask(black, square);

		//Init knight attacks table
		knight_attacks[square] = knight_attack_mask(square);

		//Init king attacks table
		king_attacks[square] = king_attack_mask(square);
		
	}
	
}

/* ----------------------SLIDER PIECES ATTACKS (MAGIC BITBOARDS)------------------- */

// GENERATE BISHOP RELEVANT OCCUPANCY MASK
BBOARD bishop_relevant_occupancy(int square) {

	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;
	
	for (rank = targetRank + 1, file = targetFile + 1; rank <= 6 && file <= 6; rank++, file++) { attacks |= (1ULL << (rank * 8 + file));	 }
	
	for (rank = targetRank - 1, file = targetFile + 1; rank >= 1 && file <= 6; rank--, file++) { attacks |= (1ULL << (rank * 8 + file)); } 

	for (rank = targetRank - 1, file = targetFile - 1; rank >= 1 && file >= 1; rank--, file--) { attacks |= (1ULL << (rank * 8 + file));}

	for (rank = targetRank + 1, file = targetFile - 1; rank <= 6 && file >= 1; rank++, file--) { attacks |= (1ULL << (rank * 8 + file));}

	return attacks; 
}

// GENERATE BISHOP ATTACK MASK BASED ON OCCUPANCY
BBOARD bishop_attack_mask(int square, BBOARD blockers) {
	BBOARD attacks = 0ULL;

	int rank, file;

	int targetRank = square / 8;
	int targetFile = square % 8;


	for (rank = targetRank + 1, file = targetFile + 1; rank <= 7 && file <= 7; rank++, file++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file <= 7; rank--, file++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank + 1, file = targetFile - 1; rank <= 7 && file >= 0; rank++, file--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	return attacks;
}

// GENERATE ROOK RELEVANT OCCUPANCY MASK
BBOARD rook_relevant_occupancy(int square) {
	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;

	for (rank = targetRank, file = targetFile + 1; file <= 6; file++) { attacks |= (1ULL << (rank * 8 + file));	}

	for (rank = targetRank, file = targetFile - 1; file >= 1; file--) { attacks |= (1ULL << (rank * 8 + file));}

	for (rank = targetRank + 1, file = targetFile; rank <= 6; rank++) { attacks |= (1ULL << (rank * 8 + file));}

	for (rank = targetRank - 1, file = targetFile; rank >= 1; rank--) { attacks |= (1ULL << (rank * 8 + file));}

	return attacks;
}

// GENERATE ROOK ATTACK MASK BASE ON OCCUPANCY
BBOARD rook_attack_mask(int square, BBOARD blockers) {
	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;

	for (rank = targetRank, file = targetFile + 1; file <= 7; file++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank, file = targetFile - 1; file >= 0; file--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank + 1, file = targetFile; rank <= 7; rank++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank - 1, file = targetFile; rank >= 0; rank--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	return attacks;
}

//Relevant occupancy bit precalcluated generator
void print_occupancy_bits(int piece) {
	if (!piece) {
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				int square = (rank * 8) + file;
				printf("%d, ", count_bits(rook_relevant_occupancy(square)));
			}
			printf("\n");
		}
	}

	if (piece) {
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				int square = (rank * 8) + file;
				printf("%d, ", count_bits(bishop_relevant_occupancy(square)));
			}
			printf("\n");
		}
	}
}


//GENERATE ALL POSSIBLE BLOCKER ARRANGEMENTS 
BBOARD gen_blocker_patterns(BBOARD attackMask) {
	
	int bitCount = count_bits(attackMask);  // Determine the number of set bits
	//printf("Number of non-zero bits: %d\n", bitCount);

	int bitIndexes[16];
	// Initialise bitIndexes dynamic array
	/* int* bitIndexes = (int*)malloc(bitCount * sizeof(int));
	if (bitIndexes == NULL) {
		printf("Memory allocation failed for bitIndexes!\n");
		return NULL;
	} */
	
	int counter = 0;

	// Store indices of set bits in bitIndexes array
	for (int i = 0; i < BOARD_SIZE; i++) {
		if (attackMask & (1ULL << i)) { bitIndexes[counter++] = i; }
	}

	// Calculate number of possible blocker patterns for piece and square
	//printf("Number of non-zero elements in bitIndexes: %d\n", bitCount);
	int numPatterns = 1ULL << bitCount;
	//printf("Expected blocker patterns: %d\n", numPatterns);

	BBOARD blockerPatterns[4096];
	/* BBOARD* blockerPatterns = (BBOARD*)malloc(numPatterns * sizeof(BBOARD));
	if (blockerPatterns == NULL) {
		printf("Memory allocation failed!\n");
		return 0ULL; // Handle memory allocation failure
	} */
	
	for (int pattern = 0; pattern < numPatterns; pattern++) {
		blockerPatterns[pattern] = 0ULL;
		for (int bitIndex = 0; bitIndex < bitCount; bitIndex++) {
			int bit = (pattern >> bitIndex) & 1;
			blockerPatterns[pattern] |= ((BBOARD)bit << bitIndexes[bitIndex]);
		}
	}

	/*
	// Loop through blockerPatterns and print entries
	 for (int i = 0; i < numPatterns; i++) {
		printBitboard(blockerPatterns[i]); 
		printf("Blocker arrangement ID: %d \n\n\n", i);
		
	} */
	
	return blockerPatterns; 
}

// Transform blockers and magic number into index
int transform(BBOARD blockers, BBOARD magic, int relevantBits) {
	return (int)((blockers * magic) >> (64 - relevantBits));
}

// MAGIC NUMBER GENERATOR
BBOARD generate_magics(int square, int relevantBits, int piece) {
	// Initialize the PRNG state
	prng_state rng;

	// Get the current time as the seed
	prng_init(&rng);

	BBOARD mask = piece ? bishop_relevant_occupancy(square) : rook_relevant_occupancy(square);
	BBOARD blockers[4096];
	BBOARD attackMask[4096]; // Attack mask after applying blockers
	BBOARD used[4096];
	BBOARD* generatedPatterns = gen_blocker_patterns(mask);
	BBOARD magic;
	int k, i, index;

	// Check if the output is not NULL before copying
	if (generatedPatterns != NULL) {
		for (int i = 0; i < (1 << relevantBits); i++) {
			blockers[i] = generatedPatterns[i]; 
			//printBitboard(blockers[i]);
			attackMask[i] = piece ? bishop_attack_mask(square, blockers[i]) : rook_attack_mask(square, blockers[i]);
		}

		for (int k = 0; k < 100000000; k++) {
			int fail = 0;
			BBOARD magic = prng_u64(&rng);
			if (count_bits((mask * magic) & 0xFF00000000000000ULL)) continue;
			for (int i = 0; i < 4096; i++) used[i] = 0ULL;

			for (int i = 0, fail = 0; !fail && i < (1 << relevantBits); i++) {
				int index = transform(blockers[i], magic, relevantBits);
				if (used[index] == 0ULL) used[index] = attackMask[i];
				else if (used[index] != attackMask[i]) fail = 1;
			}

			if (!fail) { 
				//printf("\n\n\nMAGIC NUMBER FOUND FOR %s: %llu\n\n\n", index_to_coordinate[square], magic); 
				return magic; }
		}

		printf("\n\n********FAILED*******\n\n");
		return 0ULL;

		// If gen_blocker_patterns allocates memory, make sure to free it
	}
	else {
		printf("Error generating blocker patterns!\n");
	}

	return 0ULL;
}


int main() {

	init_leaper_attacks();
	BBOARD magicNumbersRook[64];
	BBOARD magicNumbersBishop[64];
	for (int square = 0; square < 64; square++) {
	;
		magicNumbersRook[square] = generate_magics(square, rook_relevant_bitCount[square], rook);
		magicNumbersBishop[square] = generate_magics(square, bishop_relevant_bitCount[square], bishop);
		
	}
	printBitboard(magicNumbersRook[e4]);
	return 0;
}