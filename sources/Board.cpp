#include "..\headers\Board.h"

Board::Board(size_t size, size_t win_cond) {
    _size = (size > 2) ? size : 3;
    if(win_cond > 1 && win_cond <= size) {
        _win_cond = win_cond;
    } else if(win_cond <= 1) {
        _win_cond = 2;
    } else {
        _win_cond = _size;
    }

    _board = new Marker*[_size];
    for(size_t i = 0; i < _size; i++) {
        _board[i] = new Marker[_size];
        for(size_t j = 0; j < _size; j++) {
            _board[i][j] = Marker::Empty;
        }
    }

    _win_pos = new size_t[_win_cond];
}

Board::~Board() {
    if(_win_pos) {
        delete[] _win_pos;
    }

    if(_board) {
        for(size_t i = 0; i < _size; i++) {
            delete[] _board[i];
        }
        delete[] _board;
    }
}

Board::Marker Board::getMarkerAt(size_t row, size_t col) const {
    return _board[row][col];
}

void Board::setToMarker(size_t row, size_t col, Board::Marker marker) {
    _board[row][col] = marker;
}

Board::Marker Board::checkLine(size_t start_row, size_t start_col, int8_t row_offset, int8_t col_offset) {
    size_t curr_line = 1, row = start_row, col = start_col;
    _win_pos[0] = row * _size + col;
    Marker curr_marker = Marker::Empty;
    while(row < _size && col < _size) {
        if(getMarkerAt(row, col) == curr_marker && curr_marker != Marker::Empty) {
            _win_pos[curr_line] = row * _size + col;
            curr_line++;
        } else {
            curr_line = 1;
            _win_pos[0] = row * _size + col;
            curr_marker = getMarkerAt(row, col);
        }

        if(curr_line >= _win_cond) {
            return curr_marker;
        }

        row += row_offset;
        col += col_offset;
    }

    return Marker::Empty;
}

Board::Marker Board::checkWin(void) {
    Marker winner;

    for(size_t i = 0; i < _size; i++) {
        // Check rows
        winner = checkLine(i, 0, 0, 1);
        if(winner != Marker::Empty) {
            return winner;
        }

        // Check columns
        winner = checkLine(0, i, 1, 0);
        if(winner != Marker::Empty) {
            return winner;
        }

        // Check diagonals (skipping diagonals that are shorter then _win_condition)
        if(i <= _size - _win_cond) {
            winner = checkLine(_size - 1 - i, 0, -1, 1);
            if(winner != Marker::Empty) {
                return winner;
            }

            winner = checkLine(_size - 1, i, -1, 1);
            if(winner != Marker::Empty) {
                return winner;
            }

            winner = checkLine(0, i, 1, 1);
            if(winner != Marker::Empty) {
                return winner;
            }

            winner = checkLine(i, 0, 1, 1);
            if(winner != Marker::Empty) {
                return winner;
            }
        }
    }

    return Marker::Empty;
}

bool Board::checkAvailableMove(void) {
    for(size_t i = 0; i < _size; i++) {
        for(size_t j = 0; j < _size; j++) {
            if(_board[i][j] == Marker::Empty) {
                return true;
            }
        }
    }
    return false;
}