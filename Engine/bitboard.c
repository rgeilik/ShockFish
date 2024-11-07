#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "bitboard.h"
#include <windows.h>
#include <string.h>
#include <time.h>

/*
|---------------------------------------------|
|                                             |
|           IMPORTANT DEFINITIONS             |
|                                             |
|---------------------------------------------| 
*/

// CORE BITBOARDS AND FLAGS
BBOARD bitboards[12];
BBOARD occupancy[3];
int sideToMove = -1; // No side to move (illegal)
int ep = no_sq; // En Passant square (illegal)
int castlingRights;

// Attack tables for leaper pieces
extern BBOARD pawn_attacks[2][BOARD_SIZE]; // Define pawn attack table
extern BBOARD knight_attacks[BOARD_SIZE]; // Define knight attack tables
extern BBOARD king_attacks[BOARD_SIZE]; // Define king attack tables


BBOARD magicNumsRook[64] = {
	0x80002450814000,
	0x1004008900300140,
	0x2404052410200008,
	0xA10007058000280,
	0x40040800120001,
	0x200020000830410,
	0x2B00088000490402,
	0x8100009100002042,
	0x2010400340126008,
	0x400400040096410,
	0x244080068088400,
	0x98008108004020,
	0x4004201003005012,
	0x800400124020002,
	0x8000A48042000408,
	0x128316010050,
	0x8042408001611280,
	0x480102000200800,
	0x304081800248008,
	0x810500800090200,
	0x96100100802440,
	0x4000048002000080,
	0x211006000800100,
	0x2000530880,
	0x2022006200210010,
	0x8048002920080800,
	0x8064014082008800,
	0x1000A28008100008,
	0xC40A1080082400,
	0x8000080100088445,
	0xC48044080040119,
	0x4004002008014114,
	0x180004010100620,
	0xC0602082814C0030,
	0x90042408000A0081,
	0x1108100024501002,
	0x4042011040040080,
	0x9102004010500,
	0x100C481008600080,
	0x4000980108,
	0x40010800A5000,
	0x180220213003,
	0x60800C008403100,
	0x1490000486001801,
	0x80020119220001,
	0xA12088100200210,
	0x8010008028062,
	0x200801140100,
	0x50C004080190028,
	0x30401400820402,
	0x104C02410000810,
	0x8110005900044021,
	0x400010015800490,
	0x5404040100080100,
	0x4003008024420008,
	0x1000401012E4200,
	0x22030038804122,
	0x60D0008203082,
	0xC042000081129,
	0x8001200AC2001002,
	0x1CA00041012,
	0x101000C0008020B,
	0x600108004120363,
	0xC8240041002082

};

BBOARD magicNumsBishop[64] = {
	0xA020008180803880,
	0x80C010046000202,
	0x8005042002100,
	0xA01082010000,
	0x120210041002,
	0x24100084120080,
	0x24C8051124061290,
	0x20041248040208,
	0x180200214004810,
	0xB4200152004421,
	0x5180004350210004,
	0x203040408020,
	0x80404708000440,
	0x48200A110029046,
	0x4001009160280A00,
	0x40009148050209,
	0x400408200420C200,
	0x88083842022220,
	0x200480411140005,
	0x280840606020006,
	0x2400880400200406,
	0x104820890080400,
	0x8002020241008882,
	0x30A8000C000801,
	0x2404188803081280,
	0x1000010954C1,
	0x118080001420004,
	0x2080004004008,
	0x4840010802000,
	0x8000410020500220,
	0x40288158086A0,
	0x406B00302102400,
	0x802911002400024,
	0x4040822008880440,
	0x1000180040080080,
	0x400020080080081,
	0x4040400001100,
	0x400202100009000,
	0x808214400800,
	0x4101580072480,
	0x4101006C800080,
	0x920D040100860400,
	0x1000522802000128,
	0x2800048090400060,
	0x80040404100012,
	0x11402A04403081,
	0x8004080A00200,
	0x1404C82204210008,
	0x50040500200,
	0x400100404228002,
	0x80200204280A8002,
	0x400000212802820,
	0x8000004010212000,
	0x20040420A021680,
	0x2000600808948500,
	0x150208C0200C20,
	0xA8090080102240A,
	0x2002024089D00800,
	0x2200000C88045400,
	0x400020822020203,
	0x412001000C008808,
	0x41100A980100C250,
	0x80880408818,
	0x800422408402018
};

typedef struct {
	BBOARD mask;
	BBOARD magic;
} SMagic;

// Magic structures
SMagic mBishopTb1[64];
SMagic mRookTb1[64];

// Magic attack tables
BBOARD mRookAttacks[64][4096];
BBOARD mBishopAttacks[64][512];

// Zobrist keys
U64 piece_keys[12][64]; //piece_keys[piece][square]
U64 castling_keys[16];
U64 ep_keys[64];
U64 side_key;

// Position Zobrist hash key
U64 hash_key = 0ULL;




/*
|---------------------------------------------|
|                                             |
|           ZOBRIST HASHING AND KEYS          |
|                                             |
|---------------------------------------------|
*/


void gen_zobrist_keys() {
	random_state = 234893458; // Perhaps make this random later

	// Generate random U64 keys for every piece on every square
	for (int piece = P; piece <= k; piece++) {

		for (int square = 0; square < 64; square++) {
			piece_keys[piece][square] = get_random_U64_number();

		}
	}
	// Generate random U64 key for every possible castling rights combination 
	for (int castling = 0; castling < 16; castling++) {
		castling_keys[castling] = get_random_U64_number();
	}
	// Generate random U64 key for every possible en passant square 
	for (int ep = 0; ep < 16; ep++) {
		ep_keys[ep] = get_random_U64_number();
	}
	// Generate random U64 key for side
	side_key = get_random_U64_number();
}


U64 generate_hash() {
	U64 position_hash = 0ULL;

	// Perform Zobrist hashing on position hash for every piece present
	for (int piece = P; piece <= k; piece++) {
		BBOARD piece_bitboard = bitboards[piece];

		while (piece_bitboard) {
			int LSBPos = getLSB(piece_bitboard);
			position_hash ^= piece_keys[piece][LSBPos];
			pop_bit(piece_bitboard, LSBPos);
		}
	}

	// Perform Zobrist hashing on position for castling rights
	position_hash ^= castling_keys[castlingRights];

	// Perform Zobrist hashing for en passant
	if (ep != no_sq) {
		position_hash ^= ep_keys[ep];
	}

	// Perform Zobrist hashing on position if side is black
	if (sideToMove == black) { position_hash ^= side_key; }

	return position_hash;
}








/*
|----------------------------------------|
|                                        |
|           HELPER FUNCTIONS             |
|                                        |
|----------------------------------------|
*/



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


// FEN PARSER
void parseFen(char* fen) {
	int i = 0;
	int exitLoops = 0;

	// Parse piece positions and update piece bitboards
	for (int rank = 0; rank < 8; rank++) {

		for (int file = 0; file < 8; file++) {

			int square = (rank * 8) + file;

			if (ascii_to_code[fen[i]] != 0 || fen[i] == 'P') {
				int piece = ascii_to_code[fen[i]];
				set_bit(bitboards[piece], square);
				i++;
			}

			else if (isdigit(fen[i])) {
				char* endptr;
				int skip = (int)strtol(fen + i, &endptr, 10);
				file += (skip - 1);
				i++;
			}

			else if (fen[i] == '/') {
				i++;
				file--;
			}

			else { exitLoops = 1; break; }
		}

		if (exitLoops) break;

	}
	//Move to side to move field
	i++;
	// Parse side to move
	sideToMove = (fen[i] == 'w') ? white : black;
	

	// Move to castling rights field
	i += 2;

	// Parse Castling Rights field
	while (fen[i] != ' ') {

		switch (fen[i])
		{
		case 'K': castlingRights |= wk; break;
		case 'Q': castlingRights |= wq; break;
		case 'k': castlingRights |= bk; break;
		case 'q': castlingRights |= bq; break;
		}

		i++;
	}
	
	// Move to en passant field
	i++;

	if (fen[i] != '-') {
		int rank = 8 - (fen[i + 1] - '0');
		int file = fen[i] - 'a';
		ep = (rank * 8) + file;
		
		//i += 3;
	}
	
	else { ep = no_sq; /*i += 2; */ }

	

	// Populate occupancy bitboards
	for (int piece = P; piece <= K; piece++) {
		occupancy[white] |= bitboards[piece];
	}

	for (int piece = p; piece <= k; piece++) {
		occupancy[black] |= bitboards[piece];
	}

	

	occupancy[both] |= occupancy[white];
	occupancy[both] |= occupancy[black];

	// Generate hash key via Zobrist
	hash_key = generate_hash();

	
}







/*
|---------------------------------------------|
|                                             |
|           LEAPER PIECES ATTACKS             |
|                                             |
|---------------------------------------------| 
*/

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

// GENERATE PAWN QUIET MOVES



// GENERATE KNIGHT ATTACK MASK
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




/*
|---------------------------------------------|
|                                             |
|           SLIDER PIECES ATTACKS             |
|                                             |
|---------------------------------------------| 
*/

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

//Relevant occupancy bit precalculated generator
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







// INITIALISE ROOK AND BISHOP ATTACK TABLES AND MAGICS
void init_slider_attackTables(int piece) {

	BBOARD* rookPatterns;
	BBOARD* bishopPatterns;

	
	/*
	// Allocate memory for rook attacks
	mRookAttacks = malloc(BOARD_SIZE * sizeof(BBOARD*));
	for (int i = 0; i < BOARD_SIZE; i++) {
		mRookAttacks[i] = malloc(4096 * sizeof(BBOARD));
	}

	// Allocate memory for bishop attacks
	mBishopAttacks = malloc(BOARD_SIZE * sizeof(BBOARD*));
	for (int i = 0; i < BOARD_SIZE; i++) {
		mBishopAttacks[i] = malloc(512 * sizeof(BBOARD));
	} */
	
	//Initialise bishop and rook magic attack tables
	for (int square = 0; square < 64; square++) {
		rookPatterns = gen_blocker_patterns(rook_relevant_occupancy(square));
		bishopPatterns = gen_blocker_patterns(bishop_relevant_occupancy(square));
		
		

		

		// Initialise rook attack tables
		if (!piece) {
			for (int pattern = 0; pattern < (1 << rook_relevant_bitCount[square]); pattern++) {
				int magicIndex = (int)((rookPatterns[pattern] * magicNumsRook[square]) >> (64 - 12));
				mRookAttacks[square][magicIndex] = rook_attack_mask(square, rookPatterns[pattern]);
				//printBitboard(mRookAttacks[square][magicIndex]);
				//getchar();
				// Initialise Rook Magic Struct
				mRookTb1[square] = (SMagic){
					.mask = rook_relevant_occupancy(square),
					.magic = magicNumsRook[square]
				};
				
			}
		}

		// Initialise bishop attack tables
		if (piece) {
			for (int pattern = 0; pattern < (1 << bishop_relevant_bitCount[square]); pattern++) {
				int magicIndex = (int)((bishopPatterns[pattern] * magicNumsBishop[square]) >> (64 - 9));
				//printf("Magic Index: %d\n\n", magicIndex);
				//printf("Blocker pattern: \n");
				//printBitboard(bishopPatterns[pattern]);
				//getchar();
				mBishopAttacks[square][magicIndex] = bishop_attack_mask(square, bishopPatterns[pattern]);
				//printf("Attack Pattern: \n");
				//printBitboard(mBishopAttacks[square][magicIndex]);
				//getchar();
;				// Initialise Bishop Magic Struct
				mBishopTb1[square] = (SMagic){
					.mask = bishop_relevant_occupancy(square),
					.magic = magicNumsBishop[square]
				};
				
			}
		}

	}


}



// GET BISHOP, ROOK AND QUEEN ATTACKS USING MAGIC NUMBERS
BBOARD get_bishop_attacks(int square, BBOARD occupancy) {
	occupancy &= mBishopTb1[square].mask;
	occupancy *= mBishopTb1[square].magic;
	occupancy >>= (64 - 9);
	return mBishopAttacks[square][occupancy];
}

BBOARD get_rook_attacks(int square, BBOARD occupancy) {
	occupancy &= mRookTb1[square].mask;
	occupancy *= mRookTb1[square].magic;
	occupancy >>= (64 - 12);
	return mRookAttacks[square][occupancy];
}

BBOARD get_queen_attacks(int square, BBOARD occupancy) {
	BBOARD rookAttacks = get_rook_attacks(square, occupancy);
	BBOARD bishopAttacks = get_bishop_attacks(square, occupancy);

	return (rookAttacks |= bishopAttacks);
}



// Checks if square is attacked by a side
int is_square_attacked(int square, int side) {
	// Check for pawn attacks
	if ( (side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
	if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

	if ( (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) ) return 1;

	if (get_bishop_attacks(square, occupancy[both]) & ((side == white) ? bitboards[B] : bitboards[b]) ) return 1;

	if (get_rook_attacks(square, occupancy[both]) & ((side == white) ? bitboards[R] : bitboards[r]) )  return 1;

	if (get_queen_attacks(square, occupancy[both]) & ((side == white) ? bitboards[Q] : bitboards[q]) ) return 1;

	if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))  return 1;

	return 0;
}







// Macros for encoding moves and extracting information 
#define encode_move(source, target, piece, promotion, capture, doublepush, enpassant, castling) \
	(source) | \
	(target << 6) | \
	(piece << 12) | \
	(promotion << 16) | \
	(capture << 20) | \
	(doublepush << 21) | \
	(enpassant << 22) | \
	(castling << 23)    

#define get_move_source(move) (move & 0x03f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000) >> 12)
#define get_move_promotion(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) (move & 0x100000)
#define get_move_double(move) (move & 0x200000)
#define get_move_enpassant(move) (move & 0x400000)
#define get_move_castling(move) (move & 0x800000)

typedef struct {
	int moves[256];
	int moveCount;
} moveList;

// Function to add moves to move list
void add_move(moveList* list, int move) {
	list->moves[list->moveCount] = move;
	list->moveCount++;
}

// Function to print moves in algebraic notation
void print_move(int move) {
	if (!get_move_promotion(move)) {
		printf("%s%s", index_to_coordinate[get_move_source(move)], index_to_coordinate[get_move_target(move)]);
	}

	else {
		printf("%s%s%c", index_to_coordinate[get_move_source(move)], index_to_coordinate[get_move_target(move)], piece_promotions[get_move_promotion(move)]);
	}
}


// print move list
void printMoveList(moveList* move_list)
{
	// do nothing on empty move list
	if (!move_list->moveCount)
	{
		printf("\n     No move in the move list!\n");
		return;
	}

	printf("\n     move    piece     capture   double    enpassant    castling\n\n");

	// loop over moves within a move list
	for (int move_count = 0; move_count < move_list->moveCount; move_count++)
	{
		// init move
		int move = move_list->moves[move_count];

		// print move
		printf("      %s%s%c   %c         %d         %d         %d         %d\n", index_to_coordinate[get_move_source(move)],
			index_to_coordinate[get_move_target(move)],
			get_move_promotion(move) ? piece_promotions[get_move_promotion(move)] : ' ',
			ascii_pieces[get_move_piece(move)],
			get_move_capture(move) ? 1 : 0,
			get_move_double(move) ? 1 : 0,
			get_move_enpassant(move) ? 1 : 0,
			get_move_castling(move) ? 1 : 0);

	}

	// print total number of moves
	printf("\n\n     Total number of moves: %d\n\n", move_list->moveCount);

}








#define copy_position() \
BBOARD bitboards_copy[12], occupancy_copy[3]; \
int sideToMove_copy, ep_copy, castlingRights_copy; \
memcpy(bitboards_copy, bitboards, 96); \
memcpy(occupancy_copy, occupancy, 24); \
sideToMove_copy = sideToMove; ep_copy = ep; castlingRights_copy = castlingRights; \
U64 hash_key_copy = hash_key;

#define take_back() \
memcpy(bitboards, bitboards_copy, 96); \
memcpy(occupancy, occupancy_copy, 24); \
sideToMove = sideToMove_copy; ep = ep_copy; castlingRights = castlingRights_copy; \
hash_key = hash_key_copy;








int make_move(int move) {
	int source = get_move_source(move);
	int target = get_move_target(move);
	int piece = get_move_piece(move);
	int capture = get_move_capture(move);
	int promotion = get_move_promotion(move);
	int pawn_double = get_move_double(move);
	int enpassant = get_move_enpassant(move);
	int castling = get_move_castling(move);

	copy_position();

	pop_bit(bitboards[piece], source);
	set_bit(bitboards[piece], target);

	hash_key ^= piece_keys[piece][source];
	hash_key ^= piece_keys[piece][target];

	// Handle castling (source and target will be king moving 2 squares)
	if (castling) {
		switch (target) {
			case(g1): pop_bit(bitboards[R], h1); set_bit(bitboards[R], f1); hash_key ^= piece_keys[R][h1]; hash_key ^= piece_keys[R][f1]; break;
			case(c1): pop_bit(bitboards[R], a1); set_bit(bitboards[R], d1); hash_key ^= piece_keys[R][a1]; hash_key ^= piece_keys[R][d1]; break;
			case(h8): pop_bit(bitboards[r], h8); set_bit(bitboards[r], f8); hash_key ^= piece_keys[r][h8]; hash_key ^= piece_keys[r][f8]; break;
			case(c8): pop_bit(bitboards[r], a8); set_bit(bitboards[r], d8); hash_key ^= piece_keys[r][a8]; hash_key ^= piece_keys[r][d8]; break;
		}
	}

	// Handle promotions
	if (promotion) {
		pop_bit(bitboards[piece], target); // Pop pawn that has moved to 1st or 8th rank to promote
		set_bit(bitboards[promotion], target); // Set piece that pawn is being promoted to

		// Remove pawn on target square from hash key and add promoted piece on targe square to hash key
		hash_key ^= piece_keys[piece][target];
		hash_key ^= piece_keys[promotion][target];
	}

	// Handle captures
	if (capture) {
		int startpiece = (sideToMove == white) ? P : p;
		int endpiece = (sideToMove == white) ? K : k;
		
		for (; startpiece <= endpiece; startpiece++) {
			// If piece found on target square, pop and remove from hash key
			if (get_bit(bitboards[startpiece], target)) {

				pop_bit(bitboards[startpiece], target);
				hash_key ^= piece_keys[startpiece][target];
				break;
			}
		}
	}

	// Handle en passant
	if (enpassant) {

		if (sideToMove == white) {
			pop_bit(bitboards[p], target + 8);
			hash_key ^= piece_keys[p][target + 8];
			hash_key ^= ep_keys[ep_ranks[target]]; // Remove en passant from hash
		}

		else {
			pop_bit(bitboards[P], target - 8);
			hash_key ^= piece_keys[P][target - 8];
			hash_key ^= ep_keys[ep_ranks[target]]; // Remove en passant from hash
		}
		
	}

	// TO DO: Possibly look into adding an additional ep_key for no_sq, which is equal to 0 (and add to ep_ranks)
	if (ep != no_sq) hash_key ^= ep_keys[ep_ranks[ep]];
	ep = no_sq;

	// Handle double pawn moves (last because of en passant removal problems)
	if (pawn_double) {
		switch (sideToMove) {

		case(white): ep = source - 8; hash_key ^= ep_keys[ep_ranks[source - 8]]; break;
		case(black): ep = source + 8; hash_key ^= ep_keys[ep_ranks[source + 8]]; break;

		}
	}

	hash_key ^= castling_keys[castlingRights]; // Undo previous move castling rights from hash key

	castlingRights &= castling_bits[source]; // Use castling bits array to create 4-bit flag for castling on current move
	castlingRights &= castling_bits[target]; // In case rook moved

	hash_key ^= castling_keys[castlingRights]; // Update hash key with current move castling rights

	memset(occupancy, 0ULL, 24);

	for (int whitePiece = P; whitePiece <= K; whitePiece++) {
		occupancy[white] |= bitboards[whitePiece];
		occupancy[both] |= bitboards[whitePiece];
		
	}

	for (int blackPiece = p; blackPiece <= k; blackPiece++) {
		occupancy[white] |= bitboards[blackPiece];
		occupancy[both] |= bitboards[blackPiece];
	}

	

	if ((sideToMove == white) ? is_square_attacked(getLSB(bitboards[K]), black) : is_square_attacked(getLSB(bitboards[k]), white)) {
		take_back();
		return 0;
	}

	sideToMove ^= 1;
	hash_key ^= side_key; // Don't need to differ between white and black because XOR undoes itself
	

	return 1;
}


void generate_legal_moves(moveList* movelist) {

	// Generate pawn and castling moves for WHITE
	if (sideToMove == white) {
		BBOARD whitepawn_bitboard = bitboards[P];
		
		while (whitepawn_bitboard) {
			
			int source = getLSB(whitepawn_bitboard);
			int push = source - 8;
			int doublepush = source - 16;
			BBOARD captures = (pawn_attacks[white][source] & occupancy[black]);
			
			BBOARD en_passant = 0ULL;
			if (ep != no_sq) {
				set_bit(en_passant, ep);
			}
			
			// Check if square directly in front of pawn is available
			if (!get_bit(occupancy[both], push)) {
				// Check if pawn is on 7th rank to give promotion option
				if (promotion_ability[source] == 1) {

					for (int promote = N; promote < K; promote++) {
						add_move(movelist, encode_move(source, push, P, promote, 0, 0, 0, 0));
					}
				}

				// Otherwise do normal push
				else {

					add_move(movelist, encode_move(source, push, P, 0, 0, 0, 0, 0));

					// Check if pawn is on 2nd rank to give double push option
					if (!get_bit(occupancy[both], doublepush) && promotion_ability[source] == 2) { add_move(movelist, encode_move(source, doublepush, P, 0, 0, 1, 0, 0)); }
				}
			}


			// Check if pawn attacks and en passant square intersect
			if (pawn_attacks[white][source] & en_passant) {
				add_move(movelist, encode_move(source, ep, P, 0, 1, 0, 1, 0));
				pop_bit(en_passant, ep);
			}

			// Generate pawn captures
			while (captures) {
				
				int target = getLSB(captures);

				// Check if pawn is on 7th rank to give promotion option
				if (promotion_ability[source] == 1) {

					for (int promote = N; promote < K; promote++) {
						add_move(movelist, encode_move(source, push, P, promote, 1, 0, 0, 0));
					}
					pop_bit(captures, target);
				}

				// Gen normal captures
				else {
					add_move(movelist, encode_move(source, target, P, 0, 1, 0, 0, 0)); 
					pop_bit(captures, target);
				}
			}


			pop_bit(whitepawn_bitboard, source);
		}

		
		// Generate castling moves for kingside castling for white
		if (castlingRights & wk) {
			
			if (!get_bit(occupancy[both], f1) && !get_bit(occupancy[both], g1)) { 

				if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black)) {

					add_move(movelist, encode_move(e1, g1, K, 0, 0, 0, 0, 1));
				}
			}
		}

		// Generate castling moves for queenside castling for white
		if (castlingRights & wq) {
			
			if (!get_bit(occupancy[both], d1) && !get_bit(occupancy[both], c1) && !get_bit(occupancy[both], b1)) {

				if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black)) {

					add_move(movelist, encode_move(e1, c1, K, 0, 0, 0, 0, 1));
				}
			}
		}


	}




	// Generate pawn moves and castling for BLACK
	else {
		BBOARD blackpawn_bitboard = bitboards[p];
		
		while (blackpawn_bitboard) {

			int source = getLSB(blackpawn_bitboard);
			int push = source + 8;
			int doublepush = source + 16;
			BBOARD attacks = (pawn_attacks[black][source] & occupancy[white]);
			BBOARD en_passant = 0ULL;
			
			if (ep != no_sq) {
				set_bit(en_passant, ep);
			}

			// Check if square directly in front of pawn is available
			if (!get_bit(occupancy[both], push)) {
				// Check if pawn is on 2nd rank
				if (promotion_ability[source] == 2) {

					for (int promote = n; promote < k; promote++) {
						add_move(movelist, encode_move(source, push, p, promote, 0, 0, 0, 0));
					}
				}


				else {

					add_move(movelist, encode_move(source, push, p, 0, 0, 0, 0, 0));
					// Check if pawn is on 7th rank to give double push option
					if (!get_bit(occupancy[both], doublepush) && promotion_ability[source] == 1) { add_move(movelist, encode_move(source, doublepush, p, 0, 0, 1, 0, 0)); }
				}
			}



			// Check if pawn attacks and en passant square intersect
			if (pawn_attacks[black][source] & en_passant) {
				add_move(movelist, encode_move(source, ep, p, 0, 1, 0, 1, 0));
				pop_bit(en_passant, ep);
			}

			// Generate pawn captures
			while (attacks) {
				
				int target = getLSB(attacks);

				// Check if pawn is on 2nd rank and able to capture and promote
				if (promotion_ability[source] == 2) {

					for (int promote = n; promote < k; promote++) {
						add_move(movelist, encode_move(source, target, p, promote, 1, 0, 0, 0));
					}
					pop_bit(attacks, target);
				}
				
				else {
					add_move(movelist, encode_move(source, target, p, 0, 1, 0, 0, 0));

					pop_bit(attacks, target);
				}
			}


			pop_bit(blackpawn_bitboard, source);
		}



		
		

		// Generate castling moves for queenside and kingside castling for white
		if (castlingRights & bk) {

			if (!get_bit(occupancy[both], f8) && !get_bit(occupancy[both], g8)) {

				if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white)) {

					add_move(movelist, encode_move(e8, g8, k, 0, 0, 0, 0, 1));
				}
			}
		}

		if (castlingRights & bq) {

			if (!get_bit(occupancy[both], d8) && !get_bit(occupancy[both], c8) && !get_bit(occupancy[both], b8)) {

				if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white)) {

					add_move(movelist, encode_move(e8, c8, k, 0, 0, 0, 0, 1));
				}
			}
		}
	
	}


	/*-------GEN MOVES FOR EVERY OTHER PIECE----------*/

	// Generate moves for knights
	BBOARD knight_bitboard = (sideToMove == white) ? bitboards[N] : bitboards[n];

	while (knight_bitboard) {
		int source = getLSB(knight_bitboard);
		BBOARD captures = (knight_attacks[source] & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (knight_attacks[source] & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? N : n), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? N : n), 0, 1, 0, 0, 0));
			pop_bit(captures, target);
		}

		pop_bit(knight_bitboard, source);
	}


	// Generate bishop moves
	BBOARD bishop_bitboard = (sideToMove == white) ? bitboards[B] : bitboards[b];

	while (bishop_bitboard) {
		int source = getLSB(bishop_bitboard);
		BBOARD captures = (get_bishop_attacks(source, occupancy[both]) & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (get_bishop_attacks(source, occupancy[both]) & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? B : b), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? B : b), 0, 1, 0, 0, 0));
			pop_bit(captures, target);
		}

		pop_bit(bishop_bitboard, source);
	}

	// Generate rook moves
	BBOARD rook_bitboard = (sideToMove == white) ? bitboards[R] : bitboards[r];

	while (rook_bitboard) {
		int source = getLSB(rook_bitboard);
		BBOARD captures = (get_rook_attacks(source, occupancy[both]) & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (get_rook_attacks(source, occupancy[both]) & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? R : r), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? R : r), 0, 1, 0, 0, 0));
			pop_bit(captures, target);
		}

		pop_bit(rook_bitboard, source);
	}

	// Generate moves for queens
	BBOARD queen_bitboard = (sideToMove == white) ? bitboards[Q] : bitboards[q];

	while (queen_bitboard) {
		int source = getLSB(queen_bitboard);
		BBOARD captures = (get_queen_attacks(source, occupancy[both]) & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (get_queen_attacks(source, occupancy[both]) & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? Q : q), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? Q : q), 0, 1, 0, 0, 0));
			pop_bit(captures, target);
		}

		pop_bit(queen_bitboard, source);
	}

	// Generate moves for kings
	BBOARD king_bitboard = (sideToMove == white) ? bitboards[K] : bitboards[k];

	while (king_bitboard) {
		int source = getLSB(king_bitboard);
		BBOARD captures = (king_attacks[source] & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (king_attacks[source] & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? K : k), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? K : k), 0, 1, 0, 0, 0));
			pop_bit(captures, target);
		}

		pop_bit(king_bitboard, source);
	}

}


U64 Perft(int depth)
{
	moveList move_list = (moveList){
		.moves = {0},
		.moveCount = 0
	};
	int i;
	U64 nodes = 0;

	if (depth == 0)
		return 1ULL;

	generate_legal_moves(&move_list);
	for (i = 0; i < move_list.moveCount; i++) {
		copy_position();
		
		if (make_move(move_list.moves[i])) {
			nodes += Perft(depth - 1);
			take_back();
		}
		else continue;
	}
	return nodes;
}


int main() {
	clock_t start_time = clock();
	moveList move_list = (moveList){
		.moves = {0},
		.moveCount = 0
	};

	gen_zobrist_keys();
	parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	
	init_leaper_attacks(); 
	init_slider_attackTables(rook);
	init_slider_attackTables(bishop);
	
	printf("Nodes: %llu\n\n", Perft(6));
	
	// Calculate elapsed time
	clock_t end_time = clock();
	double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

	// Print the elapsed time
	printf("Elapsed time: %f seconds\n", elapsed_time);
	
	
	
	
	

	return 0;
}