#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <iostream>

enum class BoardDisplayMode {
    ASCII,
    Unicode
};

class TuiApp {
public:
    TuiApp();
    ~TuiApp();

    void run();

    // For future: set board, update diagnostics, etc.
    void set_board(const std::vector<std::vector<char>>& board);

private:
    struct notcurses* nc_;
    struct ncplane* main_plane_;
    BoardDisplayMode display_mode_{BoardDisplayMode::ASCII};
    std::vector<std::vector<char>> board_;
    std::atomic<bool> running_{true};
    std::mutex board_mutex_;

    void draw();
    void draw_board_ascii();
    void draw_board_unicode();
    void handle_input();
    void switch_display_mode();

    // VT100 helpers
    void clear_screen();
    void move_cursor(int row, int col);
};