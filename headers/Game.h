#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "Board.h"

class Game {
private:
    bool _game_over, _tie, _restart;
    size_t _board_size, _game_win_cond;
    Board::Marker _current_player;
    GLFWwindow* _window;
    Board* _game_board;

    void drawMenu(void);
    void drawBoard(void);

public:
    Game();
    ~Game();

    void run(void);
    bool restartEnabled(void) const;
};
