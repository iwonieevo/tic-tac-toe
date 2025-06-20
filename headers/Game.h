#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "Board.h"

class Game {
private:
    bool _game_over, _tie, _restart, _vs_bot, _bot_move;
    size_t _board_size, _game_win_cond;
    Board::Marker _current_player;
    GLFWwindow* _window;
    Board* _game_board;

    void drawMenu(void);
    void drawBoard(void);
    size_t chooseBestMove(Board::Marker bot_marker);
    int minimax(int depth, bool maximizing, int alpha, int beta, Board::Marker bot_marker);
    int evaluateBoard(Board::Marker bot_marker);
    int evaluateLine(Board::Marker bot_marker, size_t start_row, size_t start_col, int8_t row_offset, int8_t col_offset);

public:
    Game();
    ~Game();

    void run(void);
    bool restartEnabled(void) const;
};
