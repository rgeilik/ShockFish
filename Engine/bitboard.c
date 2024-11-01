#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "bitboard.h"
#include <windows.h>


/* --------------LEAPER PIECES ATTACKS----------------- */

extern BBOARD pawn_attacks[2][BOARD_SIZE]; // Define pawn attack table
extern BBOARD knight_attacks[BOARD_SIZE]; // Define knight attack tables
extern BBOARD king_attacks[BOARD_SIZE]; // Define king attack tables


BBOARD magicNumsRook[64] = {
	0x8a80104000800020ULL,
	0x473234BACB5E535B,
	0xE2F4B99375123507,
	0x9152D96E207B69A,
	0xC908512355A9EF21,
	0xD196C763A64EFB25,
	0x473234BACB5E535B,
	0xC908512355A9EF21,
	0xE2F4B99375123507,
	0x3B5F4E404B914492,
	0x64B40A5EA0F2F2A9,
	0x7BB0AA1A522D205B,
	0x2464E109ED9638F2,
	0x22CA05769D150A1D,
	0x7BB0AA1A522D205B,
	0x7BB0AA1A522D205B,
	0x3B5F4E404B914492,
	0x3B5F4E404B914492,
	0x7BB0AA1A522D205B,
	0x473234BACB5E535B,
	0x7BB0AA1A522D205B,
	0x64B40A5EA0F2F2A9,
	0xC908512355A9EF21,
	0x7BB0AA1A522D205B,
	0x7BB0AA1A522D205B,
	0x7BB0AA1A522D205B,
	0x7BB0AA1A522D205B,
	0xC908512355A9EF21,
	0xC908512355A9EF21,
	0xF30928E4B5645481,
	0xE2F4B99375123507,
	0x7BB0AA1A522D205B,
	0xE2F4B99375123507,
	0x7BB0AA1A522D205B,
	0xB5A65DD5DCEC0790,
	0x7BB0AA1A522D205B,
	0x3B5F4E404B914492,
	0x30FBE02DF4F57C34,
	0x473234BACB5E535B,
	0x473234BACB5E535B,
	0x7BB0AA1A522D205B,
	0x67BB4A907FA4666,
	0x473234BACB5E535B,
	0x3B5F4E404B914492,
	0x8B9BF87193838BEF,
	0x473234BACB5E535B,
	0x67BB4A907FA4666,
	0xF30928E4B5645481,
	0x67BB4A907FA4666,
	0x22CA05769D150A1D,
	0x7BB0AA1A522D205B,
	0x473234BACB5E535B,
	0x7BB0AA1A522D205B,
	0xF4877A308619283F,
	0xCB5BCF6E3AB0C387,
	0xCB5BCF6E3AB0C387,
	0x314B511F0889C297,
	0x2464E109ED9638F2,
	0xC908512355A9EF21,
	0xF30928E4B5645481

};

BBOARD magicNumsBishop[64] = {
	0xE6F3B194C557A988,
	0x443E9610D3A351EC,
	0x51C855D0C2DF22B1,
	0x57608C375A9F5E7B,
	0x8A6548E7FA9DC74B,
	0x95AE0383BA3FC130,
	0xC41CEF215704C6B8,
	0xF0E53ADC76F1096E,
	0xB92800E980010A3E,
	0xB92800E980010A3E,
	0xE935160A7E5AC541,
	0x73EA558B534D2BE,
	0x3545835B225BA24E,
	0x45EEAD855229A417,
	0x3D4CB877A5099431,
	0x3D4CB877A5099431,
	0x803A92A749066E4E,
	0x803A92A749066E4E,
	0x4AE2FE3413E2B613,
	0x5A4DAB087D92810,
	0xB8FB835E5F51971A,
	0xED4AFA3D0289A6A,
	0x5FCA9CBE648123C3,
	0x45EEAD855229A417,
	0x86491C4545619243,
	0x86491C4545619243,
	0x33DEA6BC8F50FF4A,
	0x33C4ABAC5297C76F,
	0x417A9E1EA7FE00D7,
	0x1FA842C8489A5757,
	0xF13882DEA31056A0,
	0xC46FD95337BA7735,
	0xFE96DC3C558FD017,
	0xE2F31FA57B378D3,
	0x77BC2151A335D1C4,
	0x1786AE52FE834715,
	0xADB769B7E308F60B,
	0x2FA5C6A61D10842D,
	0xFE96DC3C558FD017,
	0x83119CDAA3BB36A3,
	0x6F9938682451E07A,
	0xBE5695A2B712B87A,
	0x4B430823C3E288B6,
	0x449EFD4267D7FCDE,
	0xD2E653C50A1582C7,
	0xA61DB457AC03EB11,
	0x8A8C4BE5A6953219,
	0x78BD792743334B33,
	0xC41CEF215704C6B8,
	0xC41CEF215704C6B8,
	0x7C1F66BB69A1029A,
	0x6306D2A69F820D55,
	0xB465A75281DF1DDF,
	0x3ED0D4FDE1B76909,
	0x443E9610D3A351EC,
	0x443E9610D3A351EC,
	0xF0E53ADC76F1096E,
	0x3D4CB877A5099431,
	0xFE7AA16540E0100,
	0x443E9610D3A351EC,
	0x5A4DAB087D92810,
	0xBF81437763EC239E,
	0xB92800E980010A3E,
	0xE6F3B194C557A988
};

typedef struct {
	BBOARD mask;
	BBOARD magic;
} SMagic;

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
BBOARD* gen_blocker_patterns(BBOARD attackMask) {
	
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

	
	 BBOARD* blockerPatterns = (BBOARD*)malloc(numPatterns * sizeof(BBOARD));
	if (blockerPatterns == NULL) {
		printf("Memory allocation failed!\n");
		return 0ULL; // Handle memory allocation failure
	} 
	
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
	
	int k, i;

	// Check if the output is not NULL before copying
	if (generatedPatterns != NULL) {
		for (int i = 0; i < (1 << relevantBits); i++) {
			blockers[i] = generatedPatterns[i]; 
			//printBitboard(blockers[i]);
			attackMask[i] = piece ? bishop_attack_mask(square, blockers[i]) : rook_attack_mask(square, blockers[i]);
		}

		for (int k = 0; k < 100000000; k++) {
			
			BBOARD magic = prng_u64(&rng);
			if (count_bits((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

			memset(used, 0ULL, sizeof(used));

			int i, fail = 0;
			for (int i = 0, fail = 0; !fail && i < (1 << relevantBits); i++) {
				int index = transform(blockers[i], magic, relevantBits);
				if (used[index] == 0ULL) { used[index] = attackMask[i]; }
				else if (used[index] != attackMask[i]) { fail = 1; }
				
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
	
	
	// Declare dynamic arrays
	BBOARD** mRookAttacks;
	BBOARD** mBishopAttacks; 

	// Allocate memory
	const int NUM_SQUARES = 64;
	const int NUM_PATTERNS_ROOK = 4096;
	const int NUM_PATTERNS_BISHOP = 512;

	// Allocate memory for rook attacks
	mRookAttacks = malloc(NUM_SQUARES * sizeof(BBOARD*));
	for (int i = 0; i < NUM_SQUARES; i++) {
		mRookAttacks[i] = malloc(NUM_PATTERNS_ROOK * sizeof(BBOARD));
	}

	// Allocate memory for bishop attacks
	mBishopAttacks = malloc(NUM_SQUARES * sizeof(BBOARD*));
	for (int i = 0; i < NUM_SQUARES; i++) {
		mBishopAttacks[i] = malloc(NUM_PATTERNS_BISHOP * sizeof(BBOARD));
	}
	

	int seenMagicIndexes[4096] = { 0 };


	BBOARD* rookPatterns;
	BBOARD* bishopPatterns;
	for (int square = 0; square < 64; square++) {
		rookPatterns = gen_blocker_patterns(rook_relevant_occupancy(square));
		bishopPatterns = gen_blocker_patterns(bishop_relevant_occupancy(square));
		
		for (int pattern = 0; pattern < (1 << rook_relevant_bitCount[square]); pattern++) {
			int magicIndex = (int)((rookPatterns[pattern] * magicNumsRook[square]) >> (64 - 12));
			printf("\n\n\nMagic Index (Rook): %d            ", magicIndex);
			//printf("Square: %d          ", square);
			//printf("Pattern: %d\n\n", pattern);
			mRookAttacks[square][magicIndex] = rook_attack_mask(square, rookPatterns[pattern]);
			printf("Iteration %d\n", pattern);
			printBitboard(mRookAttacks[square][magicIndex]);
			
			if (seenMagicIndexes[magicIndex]) { printf("WARNING: MAGIC INDEX %d HAS ALREADY BEEN SEEN", magicIndex); getchar(); }
			else { seenMagicIndexes[magicIndex] = 1; }
			//if (magicIndex == 1028) {
				//getchar();
			//}	
		} 
		for (int k = 0; k < 4096; k++) { seenMagicIndexes[k] = 0; }
		printf("seenMagicIndexes has been reset!");
		getchar();
		

	} 
	
	rookPatterns = gen_blocker_patterns(rook_relevant_occupancy(a8));
	printBitboard(rookPatterns[1]);
	int index = (int)((rookPatterns[1] * magicNumsRook[a8]) >> (64 - 12));
	printf("\nMagic Index: %d\n\n", index);
	printBitboard(mRookAttacks[a8][index]); 
	//printBitboard(rook_attack_mask(a8, rookPatterns[1])); 
	

	
	// Free memory for rook attacks
	for (int i = 0; i < NUM_SQUARES; i++) {
		free(mRookAttacks[i]);
	}
	free(mRookAttacks);

	// Free memory for bishop attacks
	for (int i = 0; i < NUM_SQUARES; i++) {
		free(mBishopAttacks[i]);
	}
	free(mBishopAttacks); 

	


	/*
	BBOARD magicNumbersRook[64];
	BBOARD magicNumbersBishop[64];


	for (int square = 0; square < 64; square++) {
		;
		magicNumbersRook[square] = generate_magics(square, rook_relevant_bitCount[square], rook);
		magicNumbersBishop[square] = generate_magics(square, bishop_relevant_bitCount[square], bishop);
		printf("0x%llX, \n", magicNumbersRook[square]);
	} */

	init_leaper_attacks(); 
	
	
	return 0;
}