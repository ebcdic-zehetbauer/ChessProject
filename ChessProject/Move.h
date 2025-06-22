#pragma once
#include <string>
#include <optional>
#include "Board.h"

enum class MoveType { Normal, Capture, Castling, EnPassant, Promotion };

class Move {
public:
    int from_rank, from_file;
    int to_rank, to_file;
    MoveType type;
    std::optional<PieceType> promotion; // Only set for promotion moves

    Move(int fr, int ff, int tr, int tf, MoveType t, std::optional<PieceType> promo = std::nullopt);

    std::string to_algebraic(const Board& board) const;
};