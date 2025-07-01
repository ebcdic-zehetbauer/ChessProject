#include "TuiApp.h"
#include <iostream>
#include <limits>

TuiApp::TuiApp() {
    // Default board: empty 8x8
    board_ = std::vector<std::vector<char>>(8, std::vector<char>(8, '.'));
}

TuiApp::~TuiApp() {
    // Nothing to clean up for VT100/ASCII
}

void TuiApp::run() {
    running_ = true;
    draw();
    while (running_) {
        handle_input();
        draw();
    }
}

void TuiApp::set_board(const std::vector<std::vector<char>>& board) {
    std::lock_guard<std::mutex> lock(board_mutex_);
    board_ = board;
}

void TuiApp::draw() {
    clear_screen();
    switch (display_mode_) {
        case BoardDisplayMode::ASCII:
            draw_board_ascii();
            break;
        case BoardDisplayMode::Unicode:
            draw_board_unicode();
            break;
    }
	move_cursor(25, 1); // Move cursor to a new line below the board
    std::cout << "\nPress 'q' to quit, 'm' to switch mode.\n";
    std::cout.flush();
}

void TuiApp::draw_board_ascii() {
    std::lock_guard<std::mutex> lock(board_mutex_);
    std::cout << "  a b c d e f g h\n";
    for (int r = 0; r < 8; ++r) {
        std::cout << 8 - r << " ";
        for (int c = 0; c < 8; ++c) {
            std::cout << board_[r][c] << " ";
        }
        std::cout << 8 - r << "\n";
    }
    std::cout << "  a b c d e f g h\n";
}

void TuiApp::draw_board_unicode() {
    // Example: use Unicode chess pieces for demonstration
    // (U+2654 to U+265F for basic pieces)
    static const wchar_t* pieces[2][6] = {
        {L"\u2656", L"\u2658", L"\u2657", L"\u2655", L"\u2654", L"\u2659"}, // White
        {L"\u265C", L"\u265E", L"\u265D", L"\u265B", L"\u265A", L"\u265F"}  // Black
    };
    std::lock_guard<std::mutex> lock(board_mutex_);
    std::wcout << L"  a b c d e f g h\n";
    for (int r = 0; r < 8; ++r) {
        std::wcout << 8 - r << " ";
        for (int c = 0; c < 8; ++c) {
            // For demo, show pawns on 2nd/7th rank, dots elsewhere
            wchar_t symbol = L'.';
            if (r == 1) symbol = L'\u265F'; // Black pawn
            else if (r == 6) symbol = L'\u2659'; // White pawn
            std::wcout << symbol << " ";
        }
        std::wcout << 8 - r << L"\n";
    }
    std::wcout << L"  a b c d e f g h\n";
    std::wcout.flush();
}

void TuiApp::handle_input() {
    char ch = 0;
    std::cin >> ch;
    if (ch == 'q' || ch == 'Q') {
        running_ = false;
    } else if (ch == 'm' || ch == 'M') {
        switch_display_mode();
    }
    // Clear any extra input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void TuiApp::switch_display_mode() {
    if (display_mode_ == BoardDisplayMode::ASCII)
        display_mode_ = BoardDisplayMode::Unicode;
    else
        display_mode_ = BoardDisplayMode::ASCII;
}

void TuiApp::clear_screen() {
    // ANSI escape code to clear screen and move cursor to top-left
    std::cout << "\033[2J\033[H";
}

void TuiApp::move_cursor(int row, int col) {
    // ANSI escape code to move cursor (1-based)
    std::cout << "\033[" << row << ";" << col << "H";
}