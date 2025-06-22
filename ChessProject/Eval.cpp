#include "Eval.h"
#include "MoveGen.h"
#include "Move.h"
#include <stdexcept>

int evaluate_board(const Board& board, Color side_to_move) {
    int score = 0;
    for (int rank = 0; rank < Board::Size; ++rank) {
        for (int file = 0; file < Board::Size; ++file) {
            const auto& sq = board.at(rank, file);
            if (!sq) continue;
            int value = piece_value(sq->type);

            // Piece-square table bonus
            int pst_bonus = 0;
            if (sq->type == PieceType::Pawn) {
                int r = (sq->color == Color::White) ? rank : 7 - rank;
                pst_bonus = pawn_table[r][file];
            }
            // Add more piece-square tables for other pieces here...

            int piece_score = value + pst_bonus;
            score += (sq->color == side_to_move) ? piece_score : -piece_score;
        }
    }

    // King safety: simple penalty if king is exposed (e.g., not surrounded by pawns)
    // (You can expand this logic as needed)
    // ...

    return score;
}

// Minimax search (no alpha-beta), flexible depth, returns evaluation score
int minimax(Board& board, Color side_to_move, int depth, bool maximizingPlayer) {
    if (depth == 0) {
        return evaluate_board(board, side_to_move);
    }

    auto result = generate_legal_moves(&board, maximizingPlayer ? side_to_move : (side_to_move == Color::White ? Color::Black : Color::White));
    if (!result || result->empty()) {
        // No legal moves: checkmate or stalemate
        int eval = evaluate_board(board, side_to_move);
        // Optionally, return large negative/positive for checkmate
        return eval;
    }

    int bestEval = maximizingPlayer ? -1000000 : 1000000;

    for (const auto& move : *result) {
        Board next_board = board;
        apply_move(next_board, move);
        int eval = minimax(next_board, side_to_move, depth - 1, !maximizingPlayer);
        if (maximizingPlayer) {
            if (eval > bestEval) bestEval = eval;
        } else {
            if (eval < bestEval) bestEval = eval;
        }
    }
    return bestEval;
}

Move select_best_move(const Board& board, Color side_to_move, int depth) {
    auto result = generate_legal_moves(&board, side_to_move);
    if (!result || result->empty()) throw std::runtime_error("No legal moves");

    int bestEval = -1000000;
    Move bestMove = (*result)[0];
    for (const auto& move : *result) {
        Board next_board = board;
        apply_move(next_board, move);
        int eval = minimax(next_board, side_to_move, depth - 1, false);
        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
        }
    }
    return bestMove;
}

/*
    Note: This is plain minimax. Alpha-beta pruning is an optimization that skips
    searching branches that cannot affect the final decision, making search faster.
    For clarity, this code does not include alpha-beta pruning.
*/