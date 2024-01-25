#pragma once
#include "Game.h"
#include "Square.h"

//
// the classic game of tic tac toe
//

//
// this is the AI class
// we use a small clone here so we can recursively call minimax
//
class TicTacToeAI
{
public:
    int   _grid[3][3];
    int  _depthSearches;
    bool isBoardFull() const;
    int evaluateBoard();
    int minimax(TicTacToeAI* state, int depth, bool isMaximizingPlayer);
    int minimaxAlphaBeta(TicTacToeAI* state, int depth, bool isMaximizingPlayer, int alpha, int beta);
    int minimaxAlphaBetaSorted(TicTacToeAI* state, int depth, bool isMaximizingPlayer, int alpha, int beta);

    int ownerAt(int index ) const;
    int AICheckForWinner();
};

//
// the main game class
//
class TicTacToe : public Game
{
public:
    TicTacToe();
    ~TicTacToe();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() const override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        stopGame() override;

    TicTacToeAI* clone();
	void        updateAI() override;
    void        UpdateAISorted();

    bool        gameHasAI() override { return true; }
    BitHolder &getHolderAt(const int x, const int y) override { return _grid[y][x]; } 

private:
    Bit *       PieceForPlayer(const int playerNumber);
    Player*     ownerAt(int index ) const;

    Square      _grid[3][3];
};

