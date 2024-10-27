#include <stdio.h>
#include "utility.h"
#include "bitboard.h"


/* --------------LEAPER PIECES ATTACKS----------------- */

extern BBOARD pawn_attacks[2][64]; // Define pawn attack table
extern BBOARD knight_attacks[64]; // Define knight attack tables
extern BBOARD king_attacks[64]; // Define king attack tables

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

// Initialise attacks for leapers (pawns and knights)
void init_leaper_attacks() {
	
	for (int square = 0; square < 64; square++) {
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
BBOARD bishop_relevant_occupancy(int square) {

	BBOARD attacks = 0ULL;

	int rank, file;

	int targetRank = square / 8;
	int targetFile = square % 8;

	
	for (rank = targetRank + 1, file = targetFile + 1; rank <= 6 && file <= 6; rank++, file++) {
		attacks |= (1ULL << (rank * 8 + file));	
	}
	
	for (rank = targetRank - 1, file = targetFile + 1; rank >= 1 && file <= 6; rank--, file++) {
		attacks |= (1ULL << (rank * 8 + file));
	} 

	for (rank = targetRank - 1, file = targetFile - 1; rank >= 1 && file >= 1; rank--, file--) {
		attacks |= (1ULL << (rank * 8 + file));
	}

	for (rank = targetRank + 1, file = targetFile - 1; rank <= 6 && file >= 1; rank++, file--) {
		attacks |= (1ULL << (rank * 8 + file));
	}

	

	return attacks; 

}

BBOARD rook_relevant_occupancy(int square) {
	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;

	for (rank = targetRank, file = targetFile + 1; file <= 6; file++) {
		attacks |= (1ULL << (rank * 8 + file));
	}

	for (rank = targetRank, file = targetFile - 1; file >= 1; file--) {
		attacks |= (1ULL << (rank * 8 + file));
	}

	for (rank = targetRank + 1, file = targetFile; rank <= 6; rank++) {
		attacks |= (1ULL << (rank * 8 + file));
	}

	for (rank = targetRank - 1, file = targetFile; rank >= 1; rank--) {
		attacks |= (1ULL << (rank * 8 + file));
	}

	return attacks;
}

BBOARD gen_blocker_patterns(int piece, int square) {
	BBOARD attackMask = 0ULL;
	if (!piece) {
		attackMask = rook_relevant_occupancy(square);
	}

	else { attackMask = bishop_relevant_occupancy(square); }
	int bitIndexes[16];
	int count = 0;

	for (int i = 0; i < 64; i++) {
		if (attackMask & (1ULL << i)) { bitIndexes[count++] = i; }
	}
	printBitboard(attackMask);
	for (int i = 0; i < count; i++) { printf("%d ",bitIndexes[i]); }

	return 0ULL;
}


int main() {

	init_leaper_attacks();
	gen_blocker_patterns(rook, e4);

	
	return 0;
}