#include "..\headers\Game.h"

Game::Game() : _window(nullptr), _game_board(nullptr), _board_size(3), _game_win_cond(3), 
               _current_player(Board::Marker::X), _text_to_display(""), _current_player_text(""),
               _game_over(false), _tie(false), _restart(false), _vs_ai(false), _is_ai_turn(false), 
               _was_move_made(false), _ai_thinking(false), _ai_finished_thinking(false) {}

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

    ImGui::Checkbox("Vs AI", &_vs_ai);
    if(_vs_ai) {
        ImGui::Checkbox("AI is moving first", &_is_ai_turn);
    }

    if(ImGui::Button("Start Game")) {
        if(_vs_ai && _is_ai_turn) {
            snprintf(_current_player_text, sizeof(_text_to_display), "%s", (_current_player == Board::Marker::X) ? "X (AI)" : "O (AI)");
        } else {
            snprintf(_current_player_text, sizeof(_text_to_display), "%s", (_current_player == Board::Marker::X) ? "X" : "O");
        }
        snprintf(_text_to_display, sizeof(_text_to_display), "Current Player: %s is thinking...", _current_player_text);
        _game_board = new Board(_board_size, _game_win_cond);
    }

    ImGui::End();
}

void Game::drawBoard(void) {
    ImGui::Begin("Tic-Tac-Toe", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    ImGui::Text("First to get %d symbols in a row (either horizontally, vertically, or diagonally) wins!", _game_win_cond);

    if(_vs_ai) {
        ImGui::BeginChild("AI Times Log", ImVec2(300, _board_size * 50 + 75), true);
        ImGui::Text("AI Move Times (s)");
        if(ImGui::BeginTable("AITimesTable", 2, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Move #");
            ImGui::TableSetupColumn("Time (s)");
            ImGui::TableHeadersRow();
            for(size_t i = 0; i < _ai_timetable.size(); i++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%zu", i + 1);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.4f", _ai_timetable[i]);
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();

        ImGui::SameLine();
    }

    ImGui::BeginChild("BoardRegion", ImVec2(_board_size * 50 + 300, _board_size * 50 + 75), true);
    ImGui::Text(_text_to_display);

    // Calculate the best move in separate thread
    if(_vs_ai && _is_ai_turn && !_game_over && !_ai_thinking && !_was_move_made && !_ai_finished_thinking) {
        _ai_thinking = true;
        _ai_thread = std::thread([this]() {
            auto start = std::chrono::high_resolution_clock::now();
            _ai_move = chooseBestMove(_current_player);
            _ai_timetable.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()/1000.0);
            _ai_thinking = false;
            _ai_finished_thinking = true;
        });
        _ai_thread.detach();
    }

    // If the best move has been calculated, do it
    if(_vs_ai && _is_ai_turn && !_game_over && !_ai_thinking && !_was_move_made && _ai_finished_thinking) {
        _game_board->setToMarker(_ai_move / _board_size, _ai_move % _board_size, _current_player);
        snprintf(_current_player_text, sizeof(_text_to_display), "%s", (_current_player == Board::Marker::O) ? "X" : "O");
        _was_move_made = true;
        _ai_finished_thinking = false;
        _is_ai_turn = false;
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

            ImGui::PushStyleColor(ImGuiCol_Button, (_game_over && !_tie && isWinPos) ? ImVec4(0.4f, 0.7f, 0.4f, 1.0f) : ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (_game_over && !_tie && isWinPos) ? ImVec4(0.3f, 0.6f, 0.3f, 1.0f) : ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (_game_over && !_tie && isWinPos) ? ImVec4(0.2f, 0.5f, 0.2f, 1.0f) : ImVec4(0.15f, 0.3f, 0.5f, 1.0f));

            // If the button was pressed and it was Player's turn to make a move, update board
            if(ImGui::Button(label, ImVec2(50, 50)) && marker == Board::Marker::Empty && !_game_over && !_was_move_made && (!_vs_ai || (_vs_ai && (!_is_ai_turn || !_ai_thinking)))) {
                _game_board->setToMarker(row, col, _current_player);
                _was_move_made = true;
                if(_vs_ai) {
                    _is_ai_turn = true;
                    snprintf(_current_player_text, sizeof(_text_to_display), "%s", (_current_player == Board::Marker::O) ? "X (AI)" : "O (AI)");
                } else {
                    snprintf(_current_player_text, sizeof(_text_to_display), "%s", (_current_player == Board::Marker::O) ? "X" : "O");
                }
            }

            ImGui::PopStyleColor(3);
            ImGui::PopID();

            if(col < _board_size - 1) {
                ImGui::SameLine();
            }
        }
    }

    // Check and update the board status only if some move was made
    if(_was_move_made) {
        if (_game_board->checkWin() != Board::Marker::Empty) {
            _game_over = true;
            _tie = false;
            snprintf(_text_to_display, sizeof(_text_to_display), "Game Over! Winner: %s", _current_player_text);
        } else if(!_game_board->checkAvailableMove()) {
            _game_over = true;
            _tie = true;
            snprintf(_text_to_display, sizeof(_text_to_display), "Tie - Game Over!");
        } else {
            _current_player = (_current_player == Board::Marker::X) ? Board::Marker::O : Board::Marker::X;
            snprintf(_text_to_display, sizeof(_text_to_display), "Current Player: %s is thinking...", _current_player_text);
        }
        _was_move_made = false;
    }
    ImGui::EndChild();

    // Buttons disabled if there are any threads currently running (only making move by AI is initiating any)
    if(ImGui::Button("Restart") && !_ai_thinking) {
        _restart = true;
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    if(ImGui::Button("Exit") && !_ai_thinking) {
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
    std::vector<std::future<std::pair<int, size_t>>> futures;

    for(size_t r = 0; r < _board_size; r++) {
        for(size_t c = 0; c < _board_size; c++) {
            if(_game_board->getMarkerAt(r, c) == Board::Marker::Empty) {
                // Launching a new async thread running the code inside the lambda with access to this, r, c, and bot_marker from the current context
                futures.push_back(std::async(std::launch::async, [this, r, c, bot_marker]() {
                    Board* board_copy = new Board(*_game_board);
                    board_copy->setToMarker(r, c, bot_marker);
                    int score = minimax(board_copy, 0, false, -100000, 100000, bot_marker);
                    delete board_copy;
                    return std::make_pair(score, r * _board_size + c);
                }));
            }
        }
    }

    // Collecting results
    for (auto& fut : futures) {
        auto [score, move] = fut.get();
        if (score > bestScore) {
            bestScore = score;
            bestRow = move / _board_size;
            bestCol = move % _board_size;
        }
    }
    return bestRow * _board_size + bestCol;
}

int Game::minimax(Board* board_to_eval, int depth, bool maximizing, int alpha, int beta, Board::Marker bot_marker) {
    const int MAX_DEPTH = _game_win_cond + 2;
    Board::Marker winner = board_to_eval->checkWin(), opponent_marker = (bot_marker == Board::Marker::X) ? Board::Marker::O : Board::Marker::X;
    if(winner == bot_marker) {
        return 10000 - depth;
    } else if(winner == opponent_marker) {
        return depth - 10000;
    } else if(!board_to_eval->checkAvailableMove()) {
        return 0;
    } else if(depth >= MAX_DEPTH) {
        int score = evaluateBoard(board_to_eval, bot_marker);
        score = (score > 9999 - MAX_DEPTH) ? 9999 - MAX_DEPTH : score;
        score = (score < MAX_DEPTH - 9999) ? MAX_DEPTH - 9999 : score;
        return score;
    }

    int eval, best = (maximizing) ? -10000 : 10000;
    for(size_t r = 0; r < _board_size; r++) {
        for(size_t c = 0; c < _board_size; c++) {
            if(board_to_eval->getMarkerAt(r, c) == Board::Marker::Empty) {
                Board* to_eval_copy = new Board(*board_to_eval);
                to_eval_copy->setToMarker(r, c, (maximizing) ? bot_marker : opponent_marker);
                eval = minimax(to_eval_copy, depth + 1, !maximizing, alpha, beta, bot_marker);
                delete to_eval_copy;
                if(maximizing) {
                    best = (eval > best) ? eval : best;  // best = max(best, eval)
                    alpha = (eval > alpha) ? eval : alpha; // alpha = max(alpha, eval)
                } else {
                    best = (eval < best) ? eval : best; // best = min(best, eval)
                    beta = (eval < beta) ? eval : beta; // beta = min(beta, eval)
                }

                if(beta <= alpha) {
                    return best;
                }
            }
        }
    }
    return best;
}

int Game::evaluateBoard(Board* board_to_eval, Board::Marker bot_marker) {
    int score = 0;
    for(size_t i = 0; i < _board_size; i++) {
        // Check rows
        score += evaluateLine(board_to_eval, bot_marker, i, 0, 0, 1);

        // Check columns
        score += evaluateLine(board_to_eval, bot_marker, 0, i, 1, 0);

        // Check diagonals (skipping diagonals that are shorter then _win_condition)
        if(i <= _board_size - _game_win_cond) {
            score += evaluateLine(board_to_eval, bot_marker, _board_size - 1 - i, 0, -1, 1);
            score += evaluateLine(board_to_eval, bot_marker, _board_size - 1, i, -1, 1);
            score += evaluateLine(board_to_eval, bot_marker, 0, i, 1, 1);
            score += evaluateLine(board_to_eval, bot_marker, i, 0, 1, 1);
        }
    }

    return score;
}

int Game::evaluateLine(Board* board_to_eval, Board::Marker bot_marker, size_t start_row, size_t start_col, int8_t row_offset, int8_t col_offset) {
    int line_score = 0;
    size_t row = start_row, col = start_col, bot_c = 0, player_c = 0;
    Board::Marker m = Board::Marker::Empty;
    while(row < _board_size && col < _board_size) {
        m = board_to_eval->getMarkerAt(row, col);
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