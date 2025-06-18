#include "..\headers\Game.h"
#include <iostream>

Game::Game() : _window(nullptr), _game_board(nullptr), _board_size(3), _game_win_cond(3), _current_player(Board::Marker::X), _game_over(false), _tie(false), _restart(false) {}

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
    if(ImGui::InputInt("Board Size", (int*)&_board_size)) {
        if(_board_size < 3) {
            _board_size = 3;
        }

        if(_game_win_cond > _board_size) {
            _game_win_cond = _board_size;
        }
    }
    if(ImGui::InputInt("Winning Line Length", (int*)&_game_win_cond)) {
        if(_game_win_cond > _board_size) {
            _game_win_cond = _board_size;
        } else if(_game_win_cond < 2) {
            _game_win_cond = 2;
        }
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
            
            if(_game_over) {
                bool isWinPos = false;
                for(size_t i = 0; i < _game_win_cond; i++) {
                    if(_game_board->_win_pos[i][0] == row && _game_board->_win_pos[i][1] == col) {
                        isWinPos = true;
                        break;
                    }
                }

                if (isWinPos) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                    ImGui::Button(label, ImVec2(50, 50));
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Button(label, ImVec2(50, 50));
                }
            } else if(_tie) {
                ImGui::Button(label, ImVec2(50, 50));
            } else {
                if(ImGui::Button(label, ImVec2(50, 50)) && marker == Board::Marker::Empty && !_game_over) {
                    _game_board->setToMarker(row, col, _current_player);
                    if (_game_board->checkWin() != Board::Marker::Empty) {
                        _game_over = true;
                    } else if(!_game_board->checkAvailableMove()) {
                        _tie = true;
                    } else {
                        _current_player = (_current_player == Board::Marker::X) ? Board::Marker::O : Board::Marker::X;
                    }
                }
            }

            ImGui::PopID();
            if (col < _board_size - 1) ImGui::SameLine();
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