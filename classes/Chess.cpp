#include "Chess.h"

//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::pieceNotation(const char *state, int row, int column)
{
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }
    return state[row*8 + column];
}

const char Chess::bitToPieceNotation(int row, int column) const
{
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char *bpieces = { "?PNBRQK" };
    const char *wpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit *bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()&127];
    } else {
        notation = '0';
    }
    return notation;
}



// Convert row and column index to chess notation
std::string Chess::indexToNotation(int row, int col) 
{
    return std::string(1, 'a' + col) + std::string(1, '8' - row);
}


Chess::Chess()
{
    // Transposition zero;
    // zero.age = 0;
    // zero.alpha = 0;
    // zero.beta = 0;
    // zero.depth = 0;
    // zero.score = 0;
    // zero.hash = 0;

    // for (int i = 0; i <(zobristSize-1); i++){
    //     _zobristHashes[i] = zero;
    // }
    // AI_PLAYER = _gameOptions.AIPlayer;
    // HUMAN_PLAYER = _gameOptions.AIPlayer == 0 ? 1 : 0;
}

Chess::~Chess()
{
}

//
// make a chess piece for a player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char *pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    const char *pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "b_" : "w_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

void Chess::setUpBoard()
{
    const ChessPiece initialBoard[2][8] = {
        {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook},  // 1st Rank
        {Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn}  // 2nd Rank
    };

    _pieceSize = 64;

    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            _grid[y][x].initHolder(ImVec2((float)(100*x + 100),(float)(100*y + 100)), "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            _grid[y][x].setNotation( indexToNotation(y, x) );
        }
    }

    for (int rank=0; rank<2; rank++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            ChessPiece piece = initialBoard[rank][x];
            Bit *bit = PieceForPlayer(0, initialBoard[rank][x]);
            bit->setPosition(_grid[rank][x].getPosition());
            bit->setParent(&_grid[rank][x]);
            bit->setGameTag( piece + 128);
            _grid[rank][x].setBit( bit );

            bit = PieceForPlayer(1, initialBoard[rank][x]);
            bit->setPosition(_grid[7-rank][x].getPosition());
            bit->setParent(&_grid[7-rank][x]);
            bit->setGameTag( piece );
            _grid[7-rank][x].setBit( bit );
        }
    }

    if (gameHasAI()) {
        setAIPlayer(_gameOptions.AIPlayer);
    }

    _castleStatus = GameStarted;
    _moves = generateMoves(stateString().c_str(), _castleStatus, _lastMove, 'W');
    _depthSearches = 0;

    startGame();
    
}
 

bool Chess::canBitMoveFrom(Bit& bit, BitHolder& src)
{   
    ChessSquare &srcSquare = static_cast <ChessSquare&>(src);

    // std::string const stateStr = stateString();
    // _moves = generateMoves(stateStr.c_str(), _castleStatus, _lastMove, (getCurrentPlayer()->playerNumber() == 0) ? 'W' : 'B');

    bool canMove = false;
    for(auto move : _moves) {
        if (move.from == srcSquare.getNotation()){
            canMove = true;
            for(int y = 0; y <_gameOptions.rowY; y++) {
                for(int x = 0; x <_gameOptions.rowY; x++) {
                    ChessSquare &dstSquare = _grid[y][x];
                }
            }
        }
    }
    return canMove;
    //captures
}

bool Chess::clickedBit(Bit& bit)
{
    clearBoardHighlights();
    return true;
}

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst){
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);

    for (auto move : _moves) {
        if (move.from == srcSquare.getNotation() && move.to == dstSquare.getNotation()) {
            return true;
        }
    }
    return false;
}

void Chess::bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    const char *bpieces = "pbnrqk";
    const char *wpieces = "PBNRQK";

    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);

    _lastMove = "x-" + srcSquare.getNotation() + "-" + dstSquare.getNotation();
    _lastMove[0] = (bit.gameTag() < 128) ? wpieces[bit.gameTag() - 1] : bpieces[bit.gameTag()];

    switch (_lastMove[0]) {
        case 'K':
            _castleStatus |= WhiteKingMoved;
            break;
        case 'k':
            _castleStatus |= BlackKingMoved;
            break;
        case 'R':
            if (_lastMove[1] == 'a'){
                _castleStatus |= WhiteQueenRookMoved;
            } else if (_lastMove[1] == 'h'){
                _castleStatus |= WhiteKingRookMoved;
            }
            break;
        case 'r':
            if (_lastMove[1] == 'a'){
                _castleStatus |= BlackQueenRookMoved;
            } else if (_lastMove[1] == 'h'){
                _castleStatus |= BlackKingRookMoved;
            }
            break;
    }

    Game::bitMovedFromTo(bit, src, dst);
    _moves = generateMoves(stateString().c_str(), _castleStatus, _lastMove, (_gameOptions.currentTurnNo&1) ? 'B' : 'W');
    clearBoardHighlights();
}   

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++){
            _grid[y][x].destroyBit();
        }
    }
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString() const
{
    std::string s;
    for (int y=0; y<_gameOptions.rowY; y++) {
        for (int x=0; x<_gameOptions.rowX; x++) {
            s += bitToPieceNotation( y, x );
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{ 
    for (int y=0; y<_gameOptions.rowY; y++){
        for(int x=0; x<_gameOptions.rowX; x++){
            int index = y*_gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if(playerNumber){
                _grid[y][x].setBit(PieceForPlayer(playerNumber-1, Pawn));
            } else {
                _grid[y][x].setBit (nullptr);
            }
        }
    }
}

void Chess::addMoveIfValid(std::vector<Move>& moves, const char* state, int fromRow, int fromCol, int toRow, int toCol) {
    if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8) {
        unsigned char pieceA = pieceNotation(state, fromRow, fromCol);
        unsigned char pieceAColor = (pieceA >= 'a' && pieceA <= 'z') ? 'W' : (pieceA >= 'A' && pieceA <= 'Z') ? 'B' : '\0';
        unsigned char pieceB = pieceNotation(state, toRow, toCol);
        unsigned char pieceBColor = (pieceB >= 'a' && pieceB <= 'z') ? 'W' : (pieceB >= 'A' && pieceB <= 'Z') ? 'B' : '\0';
        if(pieceAColor != pieceBColor){
            moves.push_back({indexToNotation(fromRow, fromCol), indexToNotation(toRow, toCol)});
        }
    }
}

char Chess::oppositeColor(char color)
{
    return (color == 'W') ? 'B' : 'W';
}

void Chess::generateKnightMoves(std::vector<Move>& moves, const char*state, int row, int col){
    static const int movesRow[] = {2, 1, -1, -2, -2, -1, 1, 2};
    static const int movesCol[] = {1, 2, 2, 1, -1, -2, -2, -1};

    for (int i=0; i < 8; i++){
        int newRow = row+movesRow[i];
        int newCol = col + movesCol[i];
        addMoveIfValid(moves, state, row, col, newRow, newCol);
    }
}

void Chess::generatePawnMoves(std::vector<Move>& moves, const char*state, int row, int col, std::string lastMove, char color){
    int direction = (color == 'W') ? -1 : 1;
    int startRow = (color == 'W') ? 6 : 1;

    if (pieceNotation(state, row + direction, col) == '0'){
        addMoveIfValid(moves, state, row, col, row + direction, col);

        //two squares
        if (row == startRow && pieceNotation(state, row + 2 * direction, col) == '0'){
            addMoveIfValid(moves, state, row, col, row + 2 * direction, col);
        }
    }

    char oppositeC = oppositeColor(color);
    //capture
    for (int i = -1; i <= 1; i+=2){
        if (col + i >= 0 && col + i < 8){
            unsigned char piece = pieceNotation(state, row + direction, col + i);
            unsigned char pieceColor = (piece >= 'a' && piece <='z') ? 'W' : (piece >= 'A' && piece <= 'Z') ? 'B' : 'W';
            if(pieceColor == oppositeC){
                addMoveIfValid(moves, state, row, col, row + direction, col + i); //i maybe 1
            }
        }
    }
    if (lastMove.length() == 0){
        return;
    }
    //maybe hold
    char lastMovePiece = lastMove[0];
    int lastMoveStartRow = lastMove[3] - '0';
    int lastMoveEndRow = lastMove[6] - '0';
    int lastMoveStartCol = lastMove[2] - 'a';
    int lastMoveEndCol = lastMove[5] - 'a';

    //en passnt
    if (color == 'W' && row == 3){
        if(lastMovePiece == 'p' && lastMoveStartRow == 7 && lastMoveEndRow == 5){
            if(lastMoveEndCol == col - 1 || lastMoveEndCol == col + 1){
                addMoveIfValid(moves, state, row, col, row - 1, lastMoveEndCol);
            }
        }
    }
    else if (color == 'B' && row == 4){
        if(lastMovePiece == 'P' && lastMoveStartRow == 2 && lastMoveEndRow == 4){
            if(lastMoveEndCol == col - 1 || lastMoveEndCol == col + 1){
                addMoveIfValid(moves, state, row, col, row - 1, lastMoveEndCol);
            }
        }
    }
}

void Chess::generateLinearMoves(std::vector<Move>& moves, const char *state, int row, int col, const std::vector<std::pair<int, int>> directions){
    for(auto&dir : directions){
        int currentRow = row + dir.first;
        int currentCol = col + dir.second;
        while (currentRow >= 0 && currentRow < 8 && currentCol >= 0 && currentCol < 8){
            if (pieceNotation(state, currentRow, currentCol) != '0'){ //maybe0
                addMoveIfValid(moves, state, row, col, currentRow, currentCol);
                break;
            }
            addMoveIfValid(moves, state, row, col, currentRow, currentCol);
            currentRow += dir.first;
            currentCol += dir.second;
        }
    }
}

void Chess::generateBishopMoves(std::vector<Move>& moves, const char*state, int row, int col){
    static const std::vector<std::pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    generateLinearMoves(moves, state, row, col, directions);
}

void Chess::generateRookMoves(std::vector<Move>& moves, const char*state, int row, int col){
    static const std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    generateLinearMoves(moves, state, row, col, directions);
}

void Chess::generateQueenMoves(std::vector<Move>& moves, const char*state, int row, int col){
    generateRookMoves(moves, state, row, col);
    generateBishopMoves(moves, state, row, col);
}

void Chess::generateKingMoves(std::vector<Move>& moves, const char*state, int row, int col, int&castleStatus, char color){
    static const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };
    for(int i = 0; i < 8; i++) {
        int newRow = row + directions[i].first;
        int newCol = col + directions[i].second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8){
            addMoveIfValid(moves, state, row, col, newRow, newCol); 
        }
    }
}

std::vector<Chess::Move> Chess::generateMoves(const char *state, int&castleStatus, std::string lastMove, char color)
{
    std::vector<Move> moves;
    for(int row = 0; row < 8; row++){
        for(int col = 0; col < 8; col++){
            char piece = pieceNotation(state, row, col);
            bool correctColor = false;
            if (color == 'W'){correctColor = piece >= 'a' && piece <='z';}
            else if (color == 'B') {correctColor = piece >= 'A' && piece <= 'Z';}
            if(correctColor)
            if(piece != '0' && correctColor){
                switch(toupper(piece)){
                    case 'N':
                        generateKnightMoves(moves, state, row, col);
                        break;
                    case 'P':
                        generatePawnMoves(moves, state, row, col, lastMove, color);
                        break;
                    case 'B':
                        generateBishopMoves(moves, state, row, col);
                        break;
                    case 'R':
                        generateRookMoves(moves, state, row, col);
                        break;
                    case 'Q':
                        generateQueenMoves(moves, state, row, col);
                        break;
                    case 'K':
                        generateKingMoves(moves, state, row, col, castleStatus, color);
                        break;
                }
            }
        }
    }
    return moves;
}

int Chess::evaluateBoard(const char* stateString)
{
    const int pawnTable[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5, 
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    const int knightTable[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -40, -50
    };
    const int rookTable[64] = {
        0, 0, 0, 5, 5, 0, 0, 0,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        5, 10, 10, 10, 10, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    const int queenTable[64] ={
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        0, 0, 5, 5, 5, 5, 0, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20
    };
    const int kingTable[64] = {
        20, 30, 10, 0, 0, 10, 30, 20,
        20, 20, 0, 0, 0, 0, 20, 20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30
    };
    const int bishopTable[64] = {
        -20, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, 10, -10, -20
    };

    int score = 0;
    for (int i=0; i<64; i++){
        int blackI = (7-(i/8))*8 + (7-(i%8));
        switch (stateString[i]){
            case 'N'://knight
                score += 30;
                score += knightTable[blackI];
                break;
            case 'P': //Pawn
                score+=10;
                score += pawnTable[blackI];
                break;
            case 'B': //bishop
                score += 30;
                score += bishopTable[blackI];
                break;
            case 'R': //Rook
                score += 50;
                score += rookTable[blackI];
                break;
            case 'Q':
                score += 90;
                score += queenTable[blackI];
                break;
            case 'K':
                score += 900;
                score += kingTable[blackI];
                break;
            case 'n': 
                score -= 30;
                score -= knightTable[i];
                break;
            case 'p': //Pawn
                score -= 10;
                score -= pawnTable[i];
                break;
            case 'b': //bishop
                score -= 30;
                score -= bishopTable[i];
                break;
            case 'r': //Rook
                score -= 50;
                score -= rookTable[i];
                break;
            case 'q':
                score -= 90;
                score -= queenTable[i];
                break;
            case 'k':
                score -= 900;
                score -= kingTable[i];
                break;
        }
    }
    return score;
}

unsigned int Chess::makeAIMove(char *state, int castleStatus, Chess::Move& move){
    int fromRow = 7 - (move.from[1] - '1');
    int fromCol = move.from[0] - 'a';
    int toRow = 7 - (move.to[1] - '1');
    int toCol = move.to[0] - 'a';
    return makeAIMove(state, castleStatus, (fromRow*8)+fromCol, (toRow*8)+toCol);
}
unsigned int Chess::makeAIMove(char *state, int castleStatus, int fromIndex, int toIndex)
{
    unsigned char fromPiece = state[fromIndex];
    state[fromIndex] = '0';
    unsigned char toPiece = state[toIndex];
    state[toIndex] = fromPiece;
    return ((fromIndex<<24) | (toIndex << 16) | fromPiece << 8 | toPiece);
}
void Chess::undoAIMove(char *state, unsigned int move){
    unsigned char fromPiece = move >> 8;
    unsigned char toPiece = move & 0xFF;
    int fromIndex = (move >> 24) & 0xFF;
    int toIndex = (move >> 16) & 0xFF;
    state[fromIndex] = fromPiece;
    state[toIndex] = toPiece;
}

int Chess::minimaxAlphaBetaSorted(char* state, int depth, bool isMaximizingPlayer, int& castleStatus, int alpha, int beta) {
    // Base case: maximum depth reached or game over
    if (depth == 3) {
        return evaluateBoard(state);
    }
    _depthSearches++;
    // Generate and sort possible moves
    std::vector<Move> possibleMoves = generateMoves(state, castleStatus, _lastMove, isMaximizingPlayer ? 'W' : 'B');
    sortMoves(possibleMoves, state, isMaximizingPlayer);

    if (isMaximizingPlayer) {
        int bestVal = INT_MIN;
        for (auto& move : possibleMoves) {
            unsigned int aiMove = makeAIMove(state, castleStatus, move);
            int val = minimaxAlphaBetaSorted(state, depth +1, !isMaximizingPlayer, castleStatus, alpha, beta);
            undoAIMove(state, aiMove);

            bestVal = std::max(bestVal, val);
            alpha = std::max(alpha, bestVal);
            if (beta <= alpha) {
                break; // Beta cutoff
            }
        }
        return bestVal;
    } else {
        int bestVal = INT_MAX;
        for (auto& move : possibleMoves) {
            unsigned int aiMove = makeAIMove(state, castleStatus, move);
            int val = minimaxAlphaBetaSorted(state, depth + 1, !isMaximizingPlayer, castleStatus, alpha, beta);
            undoAIMove(state, aiMove);

            bestVal = std::min(bestVal, val);
            beta = std::min(beta, bestVal);
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
        }
        return bestVal;
    }
}
void Chess::sortMoves(std::vector<Move>& moves, const char* state, bool isMaximizingPlayer) {
    auto moveScore = [&](const Move& move) -> int {
        int score = 0;

        // Convert move notation to row and column
        int fromRow = 7 - (move.from[1] - '1');
        int fromCol = move.from[0] - 'a';
        int toRow = 7 - (move.to[1] - '1');
        int toCol = move.to[0] - 'a';

        // Capture
        char capturedPiece = pieceNotation(state, toRow, toCol);
        if (capturedPiece != '0') {
            score += (isMaximizingPlayer ? 1 : -1) * getPieceValue(capturedPiece);
        }

        // Additional heuristics can be added here

        return score;
    };

    // Sort the moves based on calculated score
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return moveScore(a) > moveScore(b);
    });
}

int Chess::getPieceValue(char piece) {
    piece = tolower(piece); // Normalize to lower case for simplicity
    switch (piece) {
        case 'p': return 10; // Pawn
        case 'n': return 30; // Knight
        case 'b': return 30; // Bishop
        case 'r': return 50; // Rook
        case 'q': return 90; // Queen
        case 'k': return 900; // King
        default: return 0;
    }
}

void Chess::updateAI()

{
    bool isMaximizingPlayer = _gameOptions.currentTurnNo&1;
    std::string stateString = Chess::stateString();
    
    char state[65];
    strcpy(state, stateString.c_str());

    std::vector<std::pair<int, std::pair<int, int>>> moves;
    std::vector<Move> possibleMoves = generateMoves(state, _castleStatus, _lastMove, isMaximizingPlayer);

    int castleStatus = _castleStatus;

    for (auto& move : _moves)
    {
        unsigned int aimove = makeAIMove(state, castleStatus, move);
        int moveScore = evaluateBoard(state);
        int fromIndex  = (aimove >> 24) & 0xFF;
        int toIndex = (aimove >> 16) & 0xFF;
        moves.push_back({moveScore, {fromIndex, toIndex}});
        undoAIMove(state, aimove);
    }
    //sort moves if ismaximizingplayer, sort descending. otherwise sort ascending
    std::sort(moves.rbegin(), moves.rend());
    int bestVal = -1000000;
    int bestMove = -1;

    for (auto& move : moves){
        _depthSearches = 0;
        auto [fromIndex, toIndex] = move.second;
        int aimove = makeAIMove(state, castleStatus, fromIndex, toIndex);
        int value = minimaxAlphaBetaSorted(state, 0, !isMaximizingPlayer, castleStatus, -1000000, 1000000);
        _gameOptions.AIDepthSearches += _depthSearches;
        if (value > bestVal){
            bestMove = aimove;
            bestVal = value;
        }
        undoAIMove(state, aimove);
    }
    // Make the best move
    if (bestMove >= 0)
    {
        int fromIndex = (bestMove >> 24) & 0xFF;
        int toIndex = (bestMove >> 16) & 0xFF;
        int fromRow = fromIndex / 8;
        int fromCol = fromIndex % 8;
        int toRow = toIndex / 8;
        int toCol = toIndex % 8;
        BitHolder& src = getHolderAt(fromCol, fromRow);
        BitHolder& dst = getHolderAt(toCol, toRow);
        Bit* bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        bitMovedFromTo(*bit, src, dst);
    }
}

