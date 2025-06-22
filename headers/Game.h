#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "Board.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <future> // for std::future and std::async used in automized async threads

class Game {
private:
    bool _game_over, _tie, _restart, _vs_ai, _is_ai_turn, _was_move_made, _ai_finished_thinking;
    std::atomic<bool> _ai_thinking;
    std::thread _ai_thread;
    std::vector<float> _ai_timetable;
    size_t _board_size, _game_win_cond, _ai_move;
    Board::Marker _current_player;
    char _text_to_display[60];
    char _current_player_text[10];
    GLFWwindow* _window;
    Board* _game_board;

    void drawMenu(void);
    void drawBoard(void);
    size_t chooseBestMove(Board::Marker bot_marker);
    int minimax(Board* board_to_eval, int depth, bool maximizing, int alpha, int beta, Board::Marker bot_marker);
    int evaluateBoard(Board* board_to_eval, Board::Marker bot_marker);
    int evaluateLine(Board* board_to_eval, Board::Marker bot_marker, size_t start_row, size_t start_col, int8_t row_offset, int8_t col_offset);

public:
    Game();
    ~Game();

    void run(void);
    bool restartEnabled(void) const;
};
