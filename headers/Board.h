#pragma once
#include <cstdint>

class Board {
public:
    Board(size_t size, size_t win_cond);
    Board(const Board& other);
    ~Board();

    enum class Marker : uint8_t {
        Empty,
        X,
        O,
        MARKER_NUM
    };

    size_t *_win_pos;

    Marker getMarkerAt(size_t row, size_t col) const;
    void setToMarker(size_t row, size_t col, Marker marker);
    Marker checkWin(void);
    bool checkAvailableMove(void);
    
private:
    size_t _size, _win_cond;
    Marker **_board;
    
    Marker checkLine(size_t start_row, size_t start_col, int8_t row_offset, int8_t col_offset);
};