#include "..\headers\Game.h"

int main() {
    Game *game = nullptr;
    bool startAgain = true;
    while(startAgain) {
        game = new Game;
        game->run();
        startAgain = game->restartEnabled();
        delete game;
    }
    return 0;
}
