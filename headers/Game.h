#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "Board.h"

class Game {
private:
    GLFWwindow* _window;
    Board* _board;
    size_t _board_size;
    size_t _win_cond;
    Board::Marker _current_player;
    bool _game_over;
    bool _tie;
    bool _restart;

    void drawMenu(void);
    void drawBoard(void);

public:
    Game();
    ~Game();

    void run(void);
};
