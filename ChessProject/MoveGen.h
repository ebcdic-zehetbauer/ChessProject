#pragma once
#include <vector>
#include <expected>
#include "Move.h"
#include "Board.h"

// Returns a list of legal moves or an error string
std::expected<std::vector<Move>, std::string>
generate_legal_moves(const Board* board, Color side_to_move);

// Applies a move to the board (modifies the board)
void apply_move(Board& board, const Move& move);

bool king_in_check(const Board& board, Color color);