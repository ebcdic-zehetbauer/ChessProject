#include "Move.h"

Move::Move(int fr, int ff, int tr, int tf, MoveType t, std::optional<PieceType> promo)
    : from_rank(fr), from_file(ff), to_rank(tr), to_file(tf), type(t), promotion(promo) {}

std::string Move::to_algebraic(const Board& /*board*/) const {
    auto square_to_str = [](int rank, int file) {
        return std::string{static_cast<char>('a' + file)} + std::to_string(rank + 1);
    };

    std::string move_str = square_to_str(from_rank, from_file) + square_to_str(to_rank, to_file);

    if (type == MoveType::Promotion && promotion.has_value()) {
        char promo_char = '?';
        switch (promotion.value()) {
            case PieceType::Queen:  promo_char = 'Q'; break;
            case PieceType::Rook:   promo_char = 'R'; break;
            case PieceType::Bishop: promo_char = 'B'; break;
            case PieceType::Knight: promo_char = 'N'; break;
            default: break;
        }
        move_str += promo_char;
    }
    return move_str;
}