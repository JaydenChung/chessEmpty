#include "Application.h"
#include "imgui/imgui.h"
#include "classes/Chess.h"
#include "classes/Zobrist.h"

namespace ClassGame {
        //
        // our global variables
        //
        Chess *game = nullptr;
        bool gameOver = false;
        int gameWinner = -1;
        bool playerColorSelected = false;
        int playerColor = 1; // Default to 1 for white
        Zobrist zobrist;

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = new Chess();
            game->setUpBoard();
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
    void RenderGame() {

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    

    ImGui::Begin("Settings");

    if (!playerColorSelected) {
        if (ImGui::Button("Play as WHITE")) {
            playerColorSelected = true;
            game = new Chess();
            game->_gameOptions.AIPlayer = 1; // AI plays as black
            game->setUpBoard();
        }

        if (ImGui::Button("Play as BLACK")) {
            playerColorSelected = true;
            game = new Chess();
            game->_gameOptions.AIPlayer = 0; // AI plays as white
            game->setUpBoard();
        }
        } else {
            // Display game information
            ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
            ImGui::Text("");
            std::string state = game->stateString();
            if (state.length() == 64) {
                for (int y=0; y<8; y++) {
                    std::string row = state.substr(y*8, 8);
                    ImGui::Text("%s", row.c_str());
                }
                ImGui::Text("");
                int64_t hash = zobrist.ZobristHash(state.c_str(), 64);
                ImGui::Text("zobrist hash: %llx", hash);
                ImGui::Text("");
                ImGui::Text("board evaluation value: %d", game->evaluateBoard(state.c_str()));
            } else {
                ImGui::Text("%s", state.c_str());
            }
            if (game->gameHasAI()) {
                ImGui::Text("");
            }
            if (gameOver) {
                ImGui::Text("Game Over!");
                ImGui::Text("Winner: %d", gameWinner);
                if (ImGui::Button("Reset Game")) {
                    game->stopGame();
                    game->setUpBoard();
                    gameOver = false;
                    gameWinner = -1;
                }
            }
        }

        ImGui::End();

        ImGui::Begin("GameWindow");
        if (playerColorSelected) {
            game->drawFrame();
        }
        ImGui::End();
    }

    // End turn is called by the game code at the end of each turn
    void EndOfTurn() {
        Player *winner = game->checkForWinner();
        if (winner) {
            gameOver = true;
            gameWinner = winner->playerNumber();
        }
        if (game->checkForDraw()) {
            gameOver = true;
            gameWinner = -1;
        }
    }
}