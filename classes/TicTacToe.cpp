#include "TicTacToe.h"
#include <algorithm>

TicTacToe::TicTacToe()
{
}

TicTacToe::~TicTacToe()
{
}

//
// make an X or an O
//
Bit* TicTacToe::PieceForPlayer(const int playerNumber)
{
    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    bit->LoadTextureFromFile(playerNumber == 1 ? "x.png" : "o.png");
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

void TicTacToe::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 3;
    _gameOptions.rowY = 3;
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            _grid[y][x].initHolder(ImVec2((float)(100*x + 100),(float)(100*y + 100)), "square.png", x, y);
        }
    }
    
    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    startGame();
}


//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool TicTacToe::actionForEmptyHolder(BitHolder &holder)
{
    if (holder.bit()) {
        return false;
    }
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());
    if (bit) {
        bit->setPosition(holder.getPosition());
        holder.setBit(bit);
        endTurn();
        return true;
    }   
    return false;
}

bool TicTacToe::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    // you can't move anything in tic tac toe
    return false;
}

bool TicTacToe::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // you can't move anything in tic tac toe
    return false;
}

//
// free all the memory used by the game on the heap
//
void TicTacToe::stopGame()
{
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            _grid[y][x].destroyBit();
        }
    }
}

//
// helper function for the winner check
//
Player* TicTacToe::ownerAt(int index ) const
{
    if (!_grid[index/_gameOptions.rowY][index%_gameOptions.rowX].bit()) {
        return nullptr;
    }
    return _grid[index/_gameOptions.rowY][index%_gameOptions.rowX].bit()->getOwner();
}

Player* TicTacToe::checkForWinner()
{
    static const int kWinningTriples[8][3] =  { {0,1,2}, {3,4,5}, {6,7,8},  // rows
                                                {0,3,6}, {1,4,7}, {2,5,8},  // cols
                                                {0,4,8}, {2,4,6} };         // diagonals
    for( int i=0; i<8; i++ ) {
        const int *triple = kWinningTriples[i];
        Player *player = ownerAt(triple[0]);
        if( player && player == ownerAt(triple[1]) && player == ownerAt(triple[2]) )
            return player;
    }
    return nullptr;
}

bool TicTacToe::checkForDraw()
{
    // check to see if the board is full
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            if (!_grid[y][x].bit()) {
                return false;
            }
        }
    }
    return true;
}

//
// state strings
//
std::string TicTacToe::initialStateString()
{
    return "000000000";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string TicTacToe::stateString() const
{
    std::string s;
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            Bit *bit = _grid[y][x].bit();
            if (bit) {
                s += std::to_string(bit->getOwner()->playerNumber()+1);
            } else {
                s += "0";
            }
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void TicTacToe::setStateString(const std::string &s)
{
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            int index = y*_gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit( PieceForPlayer(playerNumber-1) );
            } else {
                _grid[y][x].setBit( nullptr );
            }
        }
    }
}

void TicTacToe::UpdateAISorted()
{
    TicTacToeAI* state = this->clone();
    std::vector<std::pair<int, std::pair<int, int>>> moves; // Store score and move

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (!state->_grid[y][x]) {
                state->_grid[y][x] = (AI_PLAYER + 1);
                int moveScore = state->evaluateBoard(); // Evaluate the move
                moves.push_back({moveScore, {y, x}});
                state->_grid[y][x] = 0; // Undo move
            }
        }
    }

    // Sort moves: If isMaximizingPlayer, sort descending. Otherwise, sort ascending
    std::sort(moves.begin(), moves.end());
    int bestVal = -1000;
    Square* bestMove = nullptr;

    for (auto& move : moves) {
        state->_depthSearches = 0;
        auto [y, x] = move.second;
        state->_grid[y][x] = (AI_PLAYER + 1);
        int value = state->minimaxAlphaBetaSorted(state, 0, false, -1000, 1000);
        _gameOptions.AIDepthSearches += state->_depthSearches;
        // If the value of the current move is more than the best value, update best
        if (value > bestVal) {
            bestMove = &_grid[y][x];
            bestVal = value;
        }
        state->_grid[y][x] = 0; // Undo the move
    }

    delete state;
    
    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
        }
    }
}
//
// this is the function that will be called by the AI
//
void TicTacToe::updateAI() 
{
    int bestVal = -1000;
    Square* bestMove = nullptr;

    // Traverse all cells, evaluate minimax function for all empty cells
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            // Check if cell is empty
            if (!_grid[y][x].bit()) {

                // Make the move
                _grid[y][x].setBit(PieceForPlayer(AI_PLAYER));
                TicTacToeAI* newState = this->clone();
                
                int moveVal = newState->minimax(newState, 0, false); // It's the minimizing player's turn next
//                int moveVal = newState->minimaxAlphaBeta(newState, 0, false, -1000, 1000); // It's the minimizing player's turn next
                _gameOptions.AIDepthSearches += newState->_depthSearches;
                delete newState; // Don't forget to delete the cloned state!
                // Undo the move
                _grid[y][x].setBit(nullptr);

                // If the value of the current move is more than the best value, update best
                if (moveVal > bestVal) {
                    bestMove = &_grid[y][x];
                    bestVal = moveVal;
                }
            }
        }
    }

    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
        }
    }
}

//
// AI class
// this is a small class that just has a bunch of ints in it to allow us to recursively call minimax
//

TicTacToeAI* TicTacToe::clone() 
{
    TicTacToeAI* newGame = new TicTacToeAI();
    std::string gamestate = stateString();
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            int index = y*_gameOptions.rowX + x;
            int playerNumber = gamestate[index] - '0';
            newGame->_grid[y][x] = playerNumber;
            newGame->_depthSearches = 0;
        }
    }
    return newGame;
}

//
// helper function for the winner check
//
int TicTacToeAI::ownerAt(int index ) const
{
    return _grid[index/3][index%3];
}

int TicTacToeAI::AICheckForWinner()
{
    static const int kWinningTriples[8][3] =  { {0,1,2}, {3,4,5}, {6,7,8},  // rows
                                                {0,3,6}, {1,4,7}, {2,5,8},  // cols
                                                {0,4,8}, {2,4,6} };         // diagonals
    for( int i=0; i<8; i++ ) {
        const int *triple = kWinningTriples[i];
        int playerInt = ownerAt(triple[0]);
        if( playerInt != 0 && playerInt == ownerAt(triple[1]) && playerInt == ownerAt(triple[2]) )
            return playerInt;
    }
    return -1;
}
//
// Returns: positive value if AI wins, negative if human player wins, 0 for draw or undecided
//
int TicTacToeAI::evaluateBoard() 
{
    // Check for winner
    int winner = AICheckForWinner();
    if (winner == -1) {
        return 0; // No winner yet
    }
    if(winner == (AI_PLAYER+1)) 
    {
        return 10; // AI wins, positive score
    }
    return -10; // Human wins, negative score
}

//
// helper function for a draw
//
bool TicTacToeAI::isBoardFull() const
{
    for (int y=0; y<3; y++) {
        for (int x=0; x<3; x++) {
            if (!_grid[y][x]) {
                return false;
            }
        }
    }
    return true;    
}

//
// player is the current player's number (AI or human)
//
int TicTacToeAI::minimax(TicTacToeAI* state, int depth, bool isMaximizingPlayer) 
{
    int score = state->evaluateBoard();
    _depthSearches++;

    // If AI wins, human wins, or draw
    if(score == 10) return score - depth;
    if(score == -10) return score + depth;
    if(state->isBoardFull()) return 0; // Draw

    if (isMaximizingPlayer) {
        int bestVal = -1000; // Min value
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                // Check if cell is empty
                if (!state->_grid[y][x]) {
                    // Make the move
                    state->_grid[y][x] = (AI_PLAYER + 1);           // because 0 is no player, 1 is human, 2 is AI
                    bestVal = std::max(bestVal, minimax(state, depth+1, !isMaximizingPlayer));
                    // Undo the move for backtracking
                    state->_grid[y][x] = 0;
                }
            }
        }
        return bestVal;
    } else {
        int bestVal = 1000; // Max value
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                // Check if cell is empty
                if (!state->_grid[y][x]) {
                    // Make the move
                    state->_grid[y][x] = (HUMAN_PLAYER + 1);
                    bestVal = std::min(bestVal, minimax(state, depth+1, !isMaximizingPlayer));
                    // Undo the move for backtracking
                    state->_grid[y][x] = 0;
                }
            }
        }
        return bestVal;
    }
}

int TicTacToeAI::minimaxAlphaBeta(TicTacToeAI* state, int depth, bool isMaximizingPlayer, int alpha, int beta) 
{
    int score = state->evaluateBoard();
    _depthSearches++;

    // If AI wins, human wins, or draw
    if(score == 10) return score - depth;
    if(score == -10) return score + depth;
    if(state->isBoardFull()) return 0; // Draw

    if (isMaximizingPlayer) {
        int bestVal = -1000;
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                if (!state->_grid[y][x]) {
                    state->_grid[y][x] = (AI_PLAYER + 1);
                    bestVal = std::max(bestVal, minimaxAlphaBeta(state, depth + 1, false, alpha, beta));
                    state->_grid[y][x] = 0;
                    alpha = std::max(alpha, bestVal);
                    if (beta <= alpha) {
                        return bestVal; // Beta cut-off
                    }
                }
            }
        }
        return bestVal;
    } else {
        int bestVal = 1000;
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                if (!state->_grid[y][x]) {
                    state->_grid[y][x] = (HUMAN_PLAYER + 1);
                    bestVal = std::min(bestVal, minimaxAlphaBeta(state, depth + 1, true, alpha, beta));
                    state->_grid[y][x] = 0;
                    beta = std::min(beta, bestVal);
                    if (beta <= alpha) {
                        return bestVal; // Alpha cut-off
                    }
                }
            }
        }
        return bestVal;
    }
}

int TicTacToeAI::minimaxAlphaBetaSorted(TicTacToeAI* state, int depth, bool isMaximizingPlayer, int alpha, int beta) 
{
    int score = state->evaluateBoard();
    _depthSearches++;

    // If AI wins, human wins, or draw
    if(score == 10) return score - depth;
    if(score == -10) return score + depth;
    if(state->isBoardFull()) return 0; // Draw

    std::vector<std::pair<int, std::pair<int, int>>> moves; // Store score and move

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (!state->_grid[y][x]) {
                state->_grid[y][x] = (isMaximizingPlayer ? AI_PLAYER + 1 : HUMAN_PLAYER + 1);
                int moveScore = state->evaluateBoard(); // Evaluate the move with a basic score function
                moves.push_back({moveScore, {y, x}});
                state->_grid[y][x] = 0; // Undo move
            }
        }
    }

    // Sort moves: If isMaximizingPlayer, sort descending. Otherwise, sort ascending
    if (isMaximizingPlayer) {
        std::sort(moves.rbegin(), moves.rend());
    } else {
        std::sort(moves.begin(), moves.end());
    }

    if (isMaximizingPlayer) {
        int bestVal = -1000;
        for (auto& move : moves) {
            auto [y, x] = move.second;
            state->_grid[y][x] = (AI_PLAYER + 1);
            bestVal = std::max(bestVal, minimaxAlphaBetaSorted(state, depth + 1, false, alpha, beta));
            state->_grid[y][x] = 0;
            alpha = std::max(alpha, bestVal);
            if (beta <= alpha) {
                return bestVal; // Beta cut-off
            }
        }
        return bestVal;
    } else {
        int bestVal = 1000;
        for (auto& move : moves) {
            auto [y, x] = move.second;
            state->_grid[y][x] = (HUMAN_PLAYER + 1);
            bestVal = std::min(bestVal, minimaxAlphaBetaSorted(state, depth + 1, true, alpha, beta));
            state->_grid[y][x] = 0;
            beta = std::min(beta, bestVal);
            if (beta <= alpha) {
                return bestVal; // Beta cut-off
            }
        }
        return bestVal;
    }
}
