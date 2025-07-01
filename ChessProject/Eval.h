#pragma once
#include "Board.h"
#include "Move.h"
#include <vector>
#include <tuple>
#include <memory>
#include <atomic>
#include <mutex>

// Material values
constexpr int piece_value(PieceType type) {
    switch (type) {
        case PieceType::Pawn:   return 100;
        case PieceType::Knight: return 320;
        case PieceType::Bishop: return 330;
        case PieceType::Rook:   return 500;
        case PieceType::Queen:  return 900;
        case PieceType::King:   return 20000;
    }
    return 0;
}

// Example piece-square table for pawns (others can be added similarly)
constexpr int pawn_table[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};
// Add similar tables for other pieces as needed...

int evaluate_board(const Board& board, Color side_to_move);
int minimax(Board& board, Color side_to_move, int depth, bool maximizingPlayer);

Move select_best_move(const Board& board, Color side_to_move, int depth);

class MoveSelector {
public:
    MoveSelector(int num_threads = std::thread::hardware_concurrency());
    Move select_best_move(const Board& board, Color side_to_move, int depth);

private:
    int num_threads_;
};