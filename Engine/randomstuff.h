/* // Declare dynamic arrays
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
			printf("\n\n\nMagic Index (Bishop): %d            ", magicIndex);
			printf("Square: %d          ", square);
			//printf("Pattern: %d\n\n", pattern);
			mRookAttacks[square][magicIndex] = rook_attack_mask(square, rookPatterns[pattern]);
			printf("Iteration %d\n", pattern);
			printBitboard(mRookAttacks[square][magicIndex]);
			
			if (seenMagicIndexes[magicIndex]) { printf("WARNING: MAGIC INDEX %d HAS ALREADY BEEN SEEN", magicIndex); getchar(); }
			else { seenMagicIndexes[magicIndex] = 1; }
		} 
		for (int k = 0; k < 4096; k++) { seenMagicIndexes[k] = 0; }
		printf("seenMagicIndexes has been reset!");
		getchar();
		

	} */


/*// Get all pieces xraying a square
BBOARD findXrayPieces();

BBOARD xrayRookAttacks(int color, BBOARD occ, int rookSq) {
	
	BBOARD attacks = get_rook_attacks(rookSq, occ);
	
	BBOARD blockers = ((attacks & occ) & (bitboards[(color) ? R : r]) | bitboards[(color) ? Q : q]);
	
	return (attacks ^ get_rook_attacks(rookSq, occ ^ blockers));
}


BBOARD xrayBishopAttacks(int color, BBOARD occ, int bishopSq) {
	BBOARD attacks = get_bishop_attacks(bishopSq, occ);
	BBOARD possibleAttacks = bishop_relevant_occupancy(bishopSq);

	// Get piece blockers
	BBOARD pieceBlockers = (possibleAttacks & (bitboards[(color) ? B : b] | bitboards[(color) ? Q : q]));

	// Get pawn blockers that are in front based on color
	BBOARD pawnBlockers = (possibleAttacks & bitboards[(color ? P : p)]);
	if (color) { // white bishop
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL << (bishopSq + 1));
	}
	else { // black bishop
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL >> (64 - bishopSq));
	}

	BBOARD totalBlockers = (pieceBlockers | pawnBlockers);
	BBOARD furthestPieceBlockers = getFurthestBishopBlocker(bishopSq, pieceBlockers);
	BBOARD furthestPawnBlockers = getFurthestBishopBlocker(bishopSq, pawnBlockers);

	while (furthestPieceBlockers) {
		BBOARD blocker = (1ULL << getLSB(furthestPieceBlockers));
		int pieceType;
		
		int startPiece = (color == white) ? P : p;
		int endPiece = (color == white) ? Q : q;
		for (; startPiece <= endPiece; startPiece++) {
			if (blocker & bitboards[startPiece]) { pieceType = startPiece; break; }
		}
		//BBOARD xray_attacks |= (~get_bishop_attacks(bishopSq, furthestPieceBlockers) & possibleAttacks);

		if (pieceType == ((color == white) ? P : p)) {

		}

	}
	// Get attacks through pieces
	BBOARD throughPieces = (possibleAttacks);

	// Get attacks through pawns
	BBOARD throughPawns = 0ULL;
	while (pawnBlockers) {
		int pawnSq = getLSB(pawnBlockers);
		// Get bishop attacks as if the pawn wasn't there
		BBOARD withoutPawn = get_bishop_attacks(bishopSq, occ ^ (1ULL << pawnSq));
		// Only include the pawn's attack square if it's on the same diagonal
		throughPawns |= (pawn_attacks[color][pawnSq] & withoutPawn);
		pop_bit(pawnBlockers, pawnSq);
	}

	return throughPieces | throughPawns;
}


BBOARD xrayQueenAttacks(int color, BBOARD occ, int queenSq) {
	BBOARD attacks = get_queen_attacks(queenSq, occ);
	BBOARD bishopattacksonfly = bishop_attack_mask(queenSq, 0ULL);

	// Get piece blockers
	BBOARD pieceBlockers = ((attacks & occ) &
		((bitboards[(color) ? Q : q] |
			bitboards[(color) ? R : r] |
			bitboards[(color) ? B : b])));



	// Get pawn blockers that are in front
	BBOARD pawnBlockers = ((bishopattacksonfly & occ) & bitboards[(color ? P : p)]);
	if (color) {
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL << (queenSq + 1));
	}
	else {
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL >> (64 - queenSq));
	}

	

	// Get attacks through pieces
	BBOARD throughPieces = (attacks ^ get_queen_attacks(queenSq, occ ^ pieceBlockers));

	// Get attacks through pawns
	BBOARD throughPawns = 0ULL;
	while (pawnBlockers) {
		int pawnSq = getLSB(pawnBlockers); 
		
		// Get queen attacks as if the pawn wasn't there
		BBOARD withoutPawn = bishopattacksonfly;

		// Get the ray direction from queen to pawn
		int pawnFile = pawnSq % 8;
		int pawnRank = pawnSq / 8;
		int queenFile = queenSq % 8;
		int queenRank = queenSq / 8;

		// Calculate direction
		int fileDir = (pawnFile - queenFile) ? (pawnFile - queenFile) / abs(pawnFile - queenFile) : 0;
		int rankDir = (pawnRank - queenRank) ? (pawnRank - queenRank) / abs(pawnRank - queenRank) : 0;

		// Get pawn's attack squares
		BBOARD pawnAttacks = pawn_attacks[color][pawnSq];

		// Filter pawn attacks to only include the square that continues along the ray
		BBOARD validPawnAttacks = 0ULL;
		while (pawnAttacks) {
			int attackSq = getLSB(pawnAttacks);
			int attackFile = attackSq % 8;
			int attackRank = attackSq / 8;

			// Check if attack square continues along the same direction
			if ((attackFile - pawnFile) == fileDir && (attackRank - pawnRank) == rankDir) {
				validPawnAttacks |= (1ULL << attackSq);
			}
			pop_bit(pawnAttacks, attackSq);
		}

		throughPawns |= (validPawnAttacks & withoutPawn);
		pop_bit(pawnBlockers, pawnSq);
	}

	return throughPieces | throughPawns;
}*/