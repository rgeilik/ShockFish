#pragma once
#include "utility.h"
#include <stdio.h>

typedef unsigned long long BBOARD; // typedef for bitboard type

BBOARD pawn_attacks[2][64]; // Declare pawn attack table
BBOARD knight_attacks[64];   // Declare knight attack tables
BBOARD king_attacks[64];     // Declare king attack tables

BBOARD pawn_attack_mask(int color, int square);
BBOARD knight_attack_mask(int square);
BBOARD king_attack_mask(int square);
void init_leaper_attacks(); // Function to initialize leaper attacks
void printBitboard(BBOARD bitboard); // Declare print function if you need to use it in test.c
