#include "Chess.h"

enum ChessBoards {
    eWhitePawns   = 0x000000000000ff00,
    eBlackPawns   = 0x00ff000000000000,
    eWhiteKnights = 0x0000000000000042,
    eBlackKnights = 0x4200000000000000,
    eWhiteBishops = 0x0000000000000024,
    eBlackBishops = 0x2400000000000000,
    eWhiteRooks   = 0x0000000000000081,
    eBlackRooks   = 0x8100000000000000,
    eWhiteQueens  = 0x0000000000000008,
    eBlackQueens  = 0x0800000000000000,
    eWhiteKing    = 0x0000000000000010,
    eBlackKing    = 0x1000000000000000
};

void movePawn() {
    uint64_t whitePawns = eWhitePawns; // 0x000000000000FF00
    // Example: Moving a pawn from a2 to a4
    uint64_t move = (1ULL << 16) | (1ULL << 24); // a2 to a4
    whitePawns = (whitePawns ^ move); // update the white pawns bitboard

    uint64_t allWhites = eWhitePawns | eWhiteRooks | eWhiteKnights | eWhiteBishops | eWhiteQueens | eWhiteKing;
    uint64_t allBlacks = eBlackPawns | eBlackRooks | eBlackKnights | eBlackBishops | eBlackQueens | eBlackKing;
    uint64_t allPieces = allWhites | allBlacks;
}

uint64_t generateKnightMoves(uint64_t knights, uint64_t ownPieces) {
    uint64_t l1 = (knights >> 1) & 0x7f7f7f7f7f7f7f7f;
    uint64_t l2 = (knights >> 2) & 0x3f3f3f3f3f3f3f3f;
    uint64_t r1 = (knights << 1) & 0xfefefefefefefefe;
    uint64_t r2 = (knights << 2) & 0xfcfcfcfcfcfcfcfc;
    uint64_t h1 = l1 | r1;
    uint64_t h2 = l2 | r2;
    return ((h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8)) & ~ownPieces;
}

uint64_t generateBishopMoves(uint64_t bishops, uint64_t ownPieces) {
    uint64_t nw = (bishops << 9) & 0xfefefefefefefefe;
    uint64_t ne = (bishops << 7) & 0x7f7f7f7f7f7f7f7f;
    uint64_t sw = (bishops >> 7) & 0xfefefefefefefefe;
    uint64_t se = (bishops >> 9) & 0x7f7f7f7f7f7f7f7f;
    uint64_t moves = nw | ne | sw | se;
    uint64_t empty = ~ownPieces;
    uint64_t h1 = (moves << 9) & empty;
    uint64_t h2 = (moves << 7) & empty;
    uint64_t h3 = (moves >> 7) & empty;
    uint64_t h4 = (moves >> 9) & empty;
    return h1 | h2 | h3 | h4;
}

uint64_t generateWhitePawnMoves(uint64_t pawns, uint64_t allPieces) {
    uint64_t moves = (pawns << 8) & ~allPieces;  // Single step forward
    uint64_t doubleMoves = ((moves & 0x00000000FF000000) << 8) & ~allPieces;  // Double step
    return moves | doubleMoves;
}

uint64_t generateWhitePawnCaptures(uint64_t pawns, uint64_t opponentPieces) {
    uint64_t leftCaptures = (pawns << 7) & opponentPieces & 0xfefefefefefefefe;
    uint64_t rightCaptures = (pawns << 9) & opponentPieces & 0x7f7f7f7f7f7f7f7f;
    return leftCaptures | rightCaptures;
}

uint64_t generateBlackPawnMoves(uint64_t pawns, uint64_t allPieces) {
    uint64_t moves = (pawns >> 8) & ~allPieces;  // Single step forward
    uint64_t doubleMoves = ((moves & 0x000000FF00000000) >> 8) & ~allPieces;  // Double step
    return moves | doubleMoves;
}

uint64_t generateBlackPawnCaptures(uint64_t pawns, uint64_t opponentPieces) {
    uint64_t leftCaptures = (pawns >> 9) & opponentPieces & 0xfefefefefefefefe;
    uint64_t rightCaptures = (pawns >> 7) & opponentPieces & 0x7f7f7f7f7f7f7f7f;
    return leftCaptures | rightCaptures;
}

uint64_t generateRookMoves(uint64_t rooks, uint64_t ownPieces, uint64_t allPieces) {
    uint64_t moves = 0;
    uint64_t potentialMoves;
    while (rooks) {
        uint64_t rook = rooks & -rooks; // isolate the least significant bit
        // Move up
        potentialMoves = rook;
        while (potentialMoves & 0x0101010101010100) {
            potentialMoves = (potentialMoves & 0x0101010101010100) << 8;
            if (potentialMoves & allPieces) break;
            moves |= potentialMoves;
        }

        // Move down
        potentialMoves = rook;
        while (potentialMoves & 0x0080808080808080) {
            potentialMoves = (potentialMoves & 0x0080808080808080) >> 8;
            if (potentialMoves & allPieces) break;
            moves |= potentialMoves;
        }

        // Move right
        potentialMoves = rook;
        while (potentialMoves & 0x00000000000000fe) {
            potentialMoves = (potentialMoves & 0x00000000000000fe) << 1;
            if (potentialMoves & allPieces) break;
            moves |= potentialMoves;
        }

        // Move left
        potentialMoves = rook;
        while (potentialMoves & 0x007f7f7f7f7f7f7f) {
            potentialMoves = (potentialMoves & 0x007f7f7f7f7f7f7f) >> 1;
            if (potentialMoves & allPieces) break;
            moves |= potentialMoves;
        }

        rooks &= rooks - 1; // clear the least significant bit
    }
    return moves & ~ownPieces; // Exclude capturing own pieces
}
