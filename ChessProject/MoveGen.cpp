#include "MoveGen.h"
#include "Move.h"
#include "Board.h"
#include <vector>
#include <expected>

// Helper: Check if a square is on the board
constexpr bool on_board(int rank, int file) {
    return rank >= 0 && rank < Board::Size && file >= 0 && file < Board::Size;
}

// Helper: Apply a move to a board copy (basic, does not handle special moves yet)
void apply_move(Board& board, const Move& move) {
    auto piece = board.at(move.from_rank, move.from_file);
    board.at(move.from_rank, move.from_file) = std::nullopt;

    // Update castling rights if king or rook moves
    if (piece && piece->type == PieceType::King) {
        if (piece->color == Color::White) {
            board.white_kingside_castle = false;
            board.white_queenside_castle = false;
        } else {
            board.black_kingside_castle = false;
            board.black_queenside_castle = false;
        }
    }
    if (piece && piece->type == PieceType::Rook) {
        if (piece->color == Color::White) {
            if (move.from_rank == 0 && move.from_file == 0) board.white_queenside_castle = false;
            if (move.from_rank == 0 && move.from_file == 7) board.white_kingside_castle = false;
        } else {
            if (move.from_rank == 7 && move.from_file == 0) board.black_queenside_castle = false;
            if (move.from_rank == 7 && move.from_file == 7) board.black_kingside_castle = false;
        }
    }

    // Handle en passant target
    board.set_en_passant_target( std::nullopt );
    if (piece && piece->type == PieceType::Pawn && std::abs(move.to_rank - move.from_rank) == 2) {
        int ep_rank = (move.from_rank + move.to_rank) / 2;
        board.set_en_passant_target( std::make_pair(ep_rank, move.from_file));
    }

    if (move.type == MoveType::Promotion && move.promotion.has_value()) {
        board.at(move.to_rank, move.to_file) = Piece{move.promotion.value(), piece->color};
    } else if (move.type == MoveType::EnPassant) {
        board.at(move.to_rank, move.to_file) = piece;
        int captured_pawn_rank = move.from_rank;
        int captured_pawn_file = move.to_file;
        board.at(captured_pawn_rank, captured_pawn_file) = std::nullopt;
    } else if (move.type == MoveType::Castling) {
        board.at(move.to_rank, move.to_file) = piece;
        if (move.to_file == 6) { // Kingside castling
            auto rook = board.at(move.from_rank, 7);
            board.at(move.from_rank, 7) = std::nullopt;
            board.at(move.from_rank, 5) = rook;
        } else if (move.to_file == 2) { // Queenside castling
            auto rook = board.at(move.from_rank, 0);
            board.at(move.from_rank, 0) = std::nullopt;
            board.at(move.from_rank, 3) = rook;
        }
    } else {
        board.at(move.to_rank, move.to_file) = piece;
    }
}

// Helper: Check if the king of the given color is in check
bool king_in_check(const Board& board, Color color) {
    // Find king position
    int king_rank = -1, king_file = -1;
    for (int rank = 0; rank < Board::Size; ++rank) {
        for (int file = 0; file < Board::Size; ++file) {
            const auto& sq = board.at(rank, file);
            if (sq && sq->type == PieceType::King && sq->color == color) {
                king_rank = rank;
                king_file = file;
                break;
            }
        }
    }
    if (king_rank == -1) return true; // No king found, treat as in check

    Color enemy = (color == Color::White) ? Color::Black : Color::White;

    // Check for knight attacks
    const int knight_moves[8][2] = {
        {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
        {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
    };
    for (auto [dr, df] : knight_moves) {
        int r = king_rank + dr, f = king_file + df;
        if (on_board(r, f)) {
            const auto& sq = board.at(r, f);
            if (sq && sq->color == enemy && sq->type == PieceType::Knight)
                return true;
        }
    }

    // Check for pawn attacks
    int pawn_dir = (color == Color::White) ? -1 : 1;
    for (int df : {-1, 1}) {
        int r = king_rank + pawn_dir, f = king_file + df;
        if (on_board(r, f)) {
            const auto& sq = board.at(r, f);
            if (sq && sq->color == enemy && sq->type == PieceType::Pawn)
                return true;
        }
    }

    // Check for sliding piece attacks (rook/queen and bishop/queen)
    const int directions[8][2] = {
        {1,0}, {-1,0}, {0,1}, {0,-1}, // Rook/Queen
        {1,1}, {1,-1}, {-1,1}, {-1,-1} // Bishop/Queen
    };
    for (int d = 0; d < 8; ++d) {
        int dr = directions[d][0], df = directions[d][1];
        int r = king_rank + dr, f = king_file + df;
        while (on_board(r, f)) {
            const auto& sq = board.at(r, f);
            if (sq) {
                if (sq->color == enemy) {
                    if ((d < 4 && (sq->type == PieceType::Rook || sq->type == PieceType::Queen)) ||
                        (d >= 4 && (sq->type == PieceType::Bishop || sq->type == PieceType::Queen))) {
                        return true;
                    }
                    // King adjacent
                    if (sq->type == PieceType::King && (r == king_rank + dr && f == king_file + df))
                        return true;
                }
                break; // Blocked
            }
            r += dr;
            f += df;
        }
    }
    return false;
}

// ... your generate_legal_moves implementation ...
std::expected<std::vector<Move>, std::string>
generate_legal_moves(const Board* board, Color side_to_move) {
    std::vector<Move> legal_moves;

    // Add sliding piece move generation for Bishop, Rook, Queen
    auto add_sliding_moves = [&](int rank, int file, const int directions[][2], int dir_count, PieceType type) {
        for (int d = 0; d < dir_count; ++d) {
            int dr = directions[d][0], df = directions[d][1];
            int tr = rank + dr, tf = file + df;
            while (on_board(tr, tf)) {
                const auto& target = board->at(tr, tf);
                if (!target) {
                    Move move(rank, file, tr, tf, MoveType::Normal);
                    Board test_board = *board;
                    apply_move(test_board, move);
                    if (!king_in_check(test_board, side_to_move))
                        legal_moves.push_back(move);
                } else {
                    if (target->color != side_to_move) {
                        Move move(rank, file, tr, tf, MoveType::Capture);
                        Board test_board = *board;
                        apply_move(test_board, move);
                        if (!king_in_check(test_board, side_to_move))
                            legal_moves.push_back(move);
                    }
                    break; // Blocked by any piece
                }
                tr += dr;
                tf += df;
            }
        }
    };

    for (int rank = 0; rank < Board::Size; ++rank) {
        for (int file = 0; file < Board::Size; ++file) {
            const auto& sq = board->at(rank, file);
            if (!sq || sq->color != side_to_move) continue;

            PieceType type = sq->type;
            // Pawn moves
            if (type == PieceType::Pawn) {
                int dir = (side_to_move == Color::White) ? 1 : -1;
                int start_rank = (side_to_move == Color::White) ? 1 : 6;
                int promotion_rank = (side_to_move == Color::White) ? 7 : 0;

                // Forward move
                int fwd_rank = rank + dir;
                if (on_board(fwd_rank, file) && !board->at(fwd_rank, file)) {
                    // Promotion
                    if (fwd_rank == promotion_rank) {
                        for (PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                            Move move(rank, file, fwd_rank, file, MoveType::Promotion, promo);
                            Board test_board = *board;
                            apply_move(test_board, move);
                            if (!king_in_check(test_board, side_to_move))
                                legal_moves.push_back(move);
                        }
                    } else {
                        Move move(rank, file, fwd_rank, file, MoveType::Normal);
                        Board test_board = *board;
                        apply_move(test_board, move);
                        if (!king_in_check(test_board, side_to_move))
                            legal_moves.push_back(move);
                    }
                    // Double move from start
                    if (rank == start_rank && !board->at(rank + 2 * dir, file)) {
                        Move move(rank, file, rank + 2 * dir, file, MoveType::Normal);
                        Board test_board = *board;
                        apply_move(test_board, move);
                        if (!king_in_check(test_board, side_to_move))
                            legal_moves.push_back(move);
                    }
                }
                // Captures
                for (int df : {-1, 1}) {
                    int cap_file = file + df;
                    if (on_board(fwd_rank, cap_file)) {
                        const auto& target = board->at(fwd_rank, cap_file);
                        if (target && target->color != side_to_move) {
                            // Promotion capture
                            if (fwd_rank == promotion_rank) {
                                for (PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                                    Move move(rank, file, fwd_rank, cap_file, MoveType::Promotion, promo);
                                    Board test_board = *board;
                                    apply_move(test_board, move);
                                    if (!king_in_check(test_board, side_to_move))
                                        legal_moves.push_back(move);
                                }
                            } else {
                                Move move(rank, file, fwd_rank, cap_file, MoveType::Capture);
                                Board test_board = *board;
                                apply_move(test_board, move);
                                if (!king_in_check(test_board, side_to_move))
                                    legal_moves.push_back(move);
                            }
                        }
                    }
                }
                // En passant (assuming en_passant_target is a std::optional<std::pair<int, int>>)
                if (board->get_en_passant_target()) {
                    int ep_rank = board->get_en_passant_target()->first;
                    int ep_file = board->get_en_passant_target()->second;
                    if (fwd_rank == ep_rank && std::abs(file - ep_file) == 1) {
                        Move move(rank, file, ep_rank, ep_file, MoveType::EnPassant);
                        Board test_board = *board;
                        apply_move(test_board, move); // Remove captured pawn in apply_move
                        if (!king_in_check(test_board, side_to_move))
                            legal_moves.push_back(move);
                    }
                }
            }
            // Knight moves
            else if (type == PieceType::Knight) {
                const int knight_moves[8][2] = {
                    {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
                    {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
                };
                for (auto [dr, df] : knight_moves) {
                    int tr = rank + dr, tf = file + df;
                    if (!on_board(tr, tf)) continue;
                    const auto& target = board->at(tr, tf);
                    if (!target || target->color != side_to_move) {
                        Move move(rank, file, tr, tf, target ? MoveType::Capture : MoveType::Normal);
                        Board test_board = *board;
                        apply_move(test_board, move);
                        if (!king_in_check(test_board, side_to_move))
                            legal_moves.push_back(move);
                    }
                }
            }
            // Bishop moves
            else if (type == PieceType::Bishop) {
                const int bishop_dirs[4][2] = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };
                add_sliding_moves(rank, file, bishop_dirs, 4, PieceType::Bishop);
            }
            // Rook moves
            else if (type == PieceType::Rook) {
                const int rook_dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
                add_sliding_moves(rank, file, rook_dirs, 4, PieceType::Rook);
            }
            // Queen moves
            else if (type == PieceType::Queen) {
                const int queen_dirs[8][2] = { {1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1} };
                add_sliding_moves(rank, file, queen_dirs, 8, PieceType::Queen);
            }
            // King moves (non-castling)
            else if (type == PieceType::King) {
                const int king_moves[8][2] = { {1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1} };
                for (auto [dr, df] : king_moves) {
                    int tr = rank + dr, tf = file + df;
                    if (!on_board(tr, tf)) continue;
                    const auto& target = board->at(tr, tf);
                    if (!target || target->color != side_to_move) {
                        MoveType mt = target ? MoveType::Capture : MoveType::Normal;
                        Move move(rank, file, tr, tf, mt);
                        Board test_board = *board;
                        apply_move(test_board, move);
                        if (!king_in_check(test_board, side_to_move))
                            legal_moves.push_back(move);
                    }
                }
                // Castling moves
                if (!king_in_check(*board, side_to_move) && rank == (side_to_move == Color::White ? 0 : 7) && file == 4) {
                    // Kingside castling
                    bool can_castle_kingside = (side_to_move == Color::White ? board->white_kingside_castle : board->black_kingside_castle)
                        && !board->at(rank, 5) && !board->at(rank, 6)
                        && board->at(rank, 7) && board->at(rank, 7)->type == PieceType::Rook && board->at(rank, 7)->color == side_to_move;
                    if (can_castle_kingside) {
                        // Check king does not pass through or land in check
                        Board board_step1 = *board;
                        board_step1.at(rank, 5) = board_step1.at(rank, 4);
                        board_step1.at(rank, 4) = std::nullopt;
                        if (!king_in_check(board_step1, side_to_move)) {
                            Board board_step2 = board_step1;
                            board_step2.at(rank, 6) = board_step2.at(rank, 5);
                            board_step2.at(rank, 5) = std::nullopt;
                            if (!king_in_check(board_step2, side_to_move)) {
                                Move move(rank, file, rank, 6, MoveType::Castling);
                                Board test_board = *board;
                                apply_move(test_board, move);
                                if (!king_in_check(test_board, side_to_move))
                                    legal_moves.push_back(move);
                            }
                        }
                    }
                    // Queenside castling
                    bool can_castle_queenside = (side_to_move == Color::White ? board->white_queenside_castle : board->black_queenside_castle)
                        && !board->at(rank, 1) && !board->at(rank, 2) && !board->at(rank, 3)
                        && board->at(rank, 0) && board->at(rank, 0)->type == PieceType::Rook && board->at(rank, 0)->color == side_to_move;
                    if (can_castle_queenside) {
                        // Check king does not pass through or land in check
                        Board board_step1 = *board;
                        board_step1.at(rank, 3) = board_step1.at(rank, 4);
                        board_step1.at(rank, 4) = std::nullopt;
                        if (!king_in_check(board_step1, side_to_move)) {
                            Board board_step2 = board_step1;
                            board_step2.at(rank, 2) = board_step2.at(rank, 3);
                            board_step2.at(rank, 3) = std::nullopt;
                            if (!king_in_check(board_step2, side_to_move)) {
                                Move move(rank, file, rank, 2, MoveType::Castling);
                                Board test_board = *board;
                                apply_move(test_board, move);
                                if (!king_in_check(test_board, side_to_move))
                                    legal_moves.push_back(move);
                            }
                        }
                    }
                }
            }
        }
    }

    return legal_moves;
}