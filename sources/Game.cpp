#include "..\headers\Game.h"
#include <iostream>

Game::Game() : _window(nullptr), _game_board(nullptr), _board_size(3), _game_win_cond(3), _current_player(Board::Marker::X), 
               _game_over(false), _tie(false), _restart(false), _vs_bot(false), _bot_move(false) {}

Game::~Game() {
    if (_game_board) {
        delete _game_board;
    }

    if (_window) {
        glfwDestroyWindow(_window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void Game::run(void) {
    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    _window = glfwCreateWindow(1920, 1080, "Tic-Tac-Toe", nullptr, nullptr);
    if(!_window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 2.f;
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while(!glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(!_game_board) {
            drawMenu();
        } else {
            drawBoard();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(_window);
    }
}

void Game::drawMenu(void) {
    ImGui::Begin("Menu");
    ImGui::Text("Welcome to Tic-Tac-Toe!");
    ImGui::SetNextItemWidth(250);
    if(ImGui::InputInt("Board Size", (int*)&_board_size)) {
        if(_board_size < 3) {
            _board_size = 3;
        }

        if(_game_win_cond > _board_size) {
            _game_win_cond = _board_size;
        }
    }

    ImGui::SetNextItemWidth(250);
    if(ImGui::InputInt("Winning Line Length", (int*)&_game_win_cond)) {
        if(_game_win_cond > _board_size) {
            _game_win_cond = _board_size;
        } else if(_game_win_cond < 2) {
            _game_win_cond = 2;
        }
    }

    ImGui::Checkbox("Vs Bot", &_vs_bot);
    if(_vs_bot) {
        ImGui::Checkbox("Bot is moving first", &_bot_move);
    }

    if(ImGui::Button("Start Game")) {
        _game_board = new Board(_board_size, _game_win_cond);
    }

    ImGui::End();
}

void Game::drawBoard(void) {
    ImGui::Begin("Tic-Tac-Toe", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

    if(_game_over && !_tie) {
        ImGui::Text("Game Over! Winner: %s", _current_player == Board::Marker::X ? "Player X" : "Player O");
    } else if(_tie) {
        ImGui::Text("Tie - Game Over!");
    } else {
        ImGui::Text("Current Player: %s", _current_player == Board::Marker::X ? "Player X" : "Player O");
    }

    for(size_t row = 0; row < _board_size; row++) {
        for(size_t col = 0; col < _board_size; col++) {
            ImGui::PushID((int)(row * _board_size + col));
            Board::Marker marker = _game_board->getMarkerAt(row, col);
            const char* label;
            switch(marker) {
                case Board::Marker::X:
                    label = "X";
                    break;
                case Board::Marker::O:
                    label = "O";
                    break;
                default:
                    label = " ";
                    break;
            }
            
            bool isWinPos = false;
            for(size_t i = 0; i < _game_win_cond; i++) {
                if(_game_board->_win_pos[i] == row * _board_size + col) {
                    isWinPos = true;
                    break;
                }
            }

            if(_game_over && !_tie && isWinPos) {
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.4f, 0.7f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.3f, 0.5f, 1.0f));
            }

            if(ImGui::Button(label, ImVec2(50, 50)) && marker == Board::Marker::Empty && !_game_over && (!_vs_bot || (_vs_bot && !_bot_move))) {
                _game_board->setToMarker(row, col, _current_player);
                _bot_move = true;
                if (_game_board->checkWin() != Board::Marker::Empty) {
                    _game_over = true;
                    _tie = false;
                } else if(!_game_board->checkAvailableMove()) {
                    _game_over = true;
                    _tie = true;
                } else {
                    _current_player = (_current_player == Board::Marker::X) ? Board::Marker::O : Board::Marker::X;
                }
            }

            ImGui::PopStyleColor(3);

            ImGui::PopID();
            if(col < _board_size - 1) {
                ImGui::SameLine();
            }
        }
    }

    if(_vs_bot && _bot_move && !_game_over) {
        size_t pos=chooseBestMove(_current_player);
        _game_board->setToMarker(pos / _board_size, pos % _board_size, _current_player);
        _bot_move = false;
        if (_game_board->checkWin() != Board::Marker::Empty) {
            _game_over = true;
            _tie = false;
        } else if(!_game_board->checkAvailableMove()) {
            _game_over = true;
            _tie = true;
        } else {
            _current_player = (_current_player == Board::Marker::X) ? Board::Marker::O : Board::Marker::X;
        }
    }

    if(ImGui::Button("Restart")) {
        _restart = true;
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    if(ImGui::Button("Exit")) {
        _restart = false;
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    ImGui::End();
}

bool Game::restartEnabled(void) const {
    return _restart;
}

size_t Game::chooseBestMove(Board::Marker bot_marker) {
    int bestScore = -100000;
    size_t bestRow, bestCol;

    for(size_t r = 0; r < _board_size; r++) {
        for(size_t c = 0; c < _board_size; c++) {
            if(_game_board->getMarkerAt(r, c) == Board::Marker::Empty) {
                _game_board->setToMarker(r, c, bot_marker);
                int score = minimax(0, false, -100000, 100000, bot_marker);
                _game_board->setToMarker(r, c, Board::Marker::Empty);
                if(score > bestScore) {
                    bestScore = score;
                    bestRow = r;
                    bestCol = c;
                }
            }
        }
    }
    return bestRow * _board_size + bestCol;
}

int Game::minimax(int depth, bool maximizing, int alpha, int beta, Board::Marker bot_marker) {
    const int MAX_DEPTH = 4;
    Board::Marker winner = _game_board->checkWin(), opponent_marker = (bot_marker == Board::Marker::X) ? Board::Marker::O : Board::Marker::X;
    if(winner == bot_marker) {
        return 10000 - depth;
    } else if(winner == opponent_marker) {
        return depth - 10000;
    } else if(!_game_board->checkAvailableMove()) {
        return 0;
    } else if(depth >= MAX_DEPTH) {
        int score = evaluateBoard(bot_marker);
        score = (score > 9999) ? 9999 : score;
        score = (score < -9999) ? -9999 : score;
        return score;
    }

    int eval, mEval = (maximizing) ? -10000 : 10000;
    for(size_t r = 0; r < _board_size; r++) {
        for(size_t c = 0; c < _board_size; c++) {
            if(_game_board->getMarkerAt(r, c) == Board::Marker::Empty) {
                _game_board->setToMarker(r, c, (maximizing) ? bot_marker : opponent_marker);
                eval = minimax(depth + 1, !maximizing, alpha, beta, bot_marker);
                _game_board->setToMarker(r, c, Board::Marker::Empty);

                if(maximizing) {
                    mEval = (eval > mEval) ? eval : mEval; 
                    alpha = (eval > alpha) ? alpha : mEval; 
                } else {
                    mEval = (eval < mEval) ? eval : mEval; 
                    beta = (eval < beta) ? beta : mEval; 
                }

                if(beta <= alpha) {
                    return mEval;
                }
            }
        }
    }
    return mEval;
}

int Game::evaluateBoard(Board::Marker bot_marker) {
    int score = 0;
    for(size_t i = 0; i < _board_size; i++) {
        // Check rows
        score += evaluateLine(bot_marker, i, 0, 0, 1);

        // Check columns
        score += evaluateLine(bot_marker, 0, i, 1, 0);

        // Check diagonals (skipping diagonals that are shorter then _win_condition)
        if(i <= _board_size - _game_win_cond) {
            score += evaluateLine(bot_marker, _board_size - 1 - i, 0, -1, 1);
            score += evaluateLine(bot_marker, _board_size - 1, i, -1, 1);
            score += evaluateLine(bot_marker, 0, i, 1, 1);
            score += evaluateLine(bot_marker, i, 0, 1, 1);
        }
    }

    return score;
}

int Game::evaluateLine(Board::Marker bot_marker, size_t start_row, size_t start_col, int8_t row_offset, int8_t col_offset) {
    int line_score = 0;
    size_t row = start_row, col = start_col, bot_c = 0, player_c = 0;
    Board::Marker m = Board::Marker::Empty;
    while(row < _board_size && col < _board_size) {
        m = _game_board->getMarkerAt(row, col);
        if(m == bot_marker) {
            bot_c++;
        } else if(m != Board::Marker::Empty) {
            player_c++;
        }

        row += row_offset;
        col += col_offset;
    }

    if(bot_c > 0 && player_c == 0) {
        if(bot_c == _game_win_cond - 1) {
            return 1000;
        } else if(bot_c == _game_win_cond - 2) {
            return 10;
        } else {
            return bot_c;
        }
    } else if(bot_c == 0 && player_c > 0) {
        if(player_c == _game_win_cond - 1) {
            return -1000;
        } else if(player_c == _game_win_cond - 2) {
            return -10;
        } else {
            return -player_c;
        }
    }

    return line_score;
}