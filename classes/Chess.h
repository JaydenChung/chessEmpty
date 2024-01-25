#pragma once
#include "Game.h"
#include <algorithm>
#include "ChessSquare.h"

//
// the classic game of chess
//
enum ChessPiece {
    Pawn = 1,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};


enum Castling {
    GameStarted = 0,
    WhiteKingRookMoved = 0x1,
    WhiteQueenRookMoved = 0x2,
    BlackKingRookMoved = 0x4,
    BlackQueenRookMoved = 0x8,
    WhiteKingMoved = 0x10,
    BlackKingMoved = 0x20
};

//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    struct Move {
        std::string from;
        std::string to;
    };

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() const override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override {return false; }
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;

    bool	    clickedBit(Bit& bit) override;
    void        stopGame() override;
    int         evaluateBoard(const char* stateString);

    void        updateAI() override;
    bool        gameHasAI() override {return true;}
    int         minimaxAlphaBetaSorted(char* state, int depth, bool isMaximizingPlayer, int& castleStatus, int alpha, int beta);

    BitHolder &getHolderAt(const int x, const int y) override { return _grid[y][x]; } 
private:
    int     _castleStatus;
    int     _depthSearches;

    Bit *               PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player*             ownerAt(int index) const;
    void                addMoveIfValid(std::vector<Move>& moves, const char* state, int fromRow, int fromCol, int toRow, int toCol);
    std::string         indexToNotation(int row, int col);
    static const char   pieceNotation(const char *state, int row, int column);
    void                generateKnightMoves(std::vector<Move>& moves, const char*state, int row, int col);
    void                generatePawnMoves(std::vector<Move>& moves, const char*state, int row, int col, std::string lastMove, char color);
    void                generateLinearMoves(std::vector<Move>& moves, const char *state, int row, int col, const std::vector<std::pair<int, int>> directions);
    void                generateBishopMoves(std::vector<Move>& moves, const char*state, int row, int col);
    void                generateRookMoves(std::vector<Move>& moves, const char*state, int row, int col);
    void                generateQueenMoves(std::vector<Move>& moves, const char*state, int row, int col);
    void                generateKingMoves(std::vector<Move>& moves, const char*state, int row, int col, int&castleStatus, char color);
    char                oppositeColor(char color);

    std::vector<Chess::Move> generateMoves(const char *state, int&castleStatus, std::string lastMove, char color);

    unsigned int        makeAIMove(char *state, int castleStatus, Chess::Move& move);
    unsigned int        makeAIMove(char *state, int castleStatus, int fromIndex, int toIndex);
    void                undoAIMove(char *state, unsigned int move);

    const char  bitToPieceNotation(int row, int column) const;

    ChessSquare         _grid[8][8];
    int     _pieceSize;

    std::vector<Chess::Move> _moves;

    int         getPieceValue(char piece);
    void        sortMoves(std::vector<Move>& moves, const char* state, bool isMaximizingPlayer);

};  