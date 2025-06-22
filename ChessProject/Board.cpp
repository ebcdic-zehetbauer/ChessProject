#include "Board.h"
#include <format>
#include <sstream>
#include <string>
#include <cctype>

Board::Board() {
    setup_initial_position();
}

void Board::setup_initial_position() {
    clear_board();
    for (int file = 0; file < Size; ++file) {
        squares_[1][file] = Piece{ PieceType::Pawn, Color::White };
        squares_[6][file] = Piece{ PieceType::Pawn, Color::Black };
    }
    squares_[0][0] = squares_[0][7] = Piece{ PieceType::Rook, Color::White };
    squares_[0][1] = squares_[0][6] = Piece{ PieceType::Knight, Color::White };
    squares_[0][2] = squares_[0][5] = Piece{ PieceType::Bishop, Color::White };
    squares_[0][3] = Piece{ PieceType::Queen, Color::White };
    squares_[0][4] = Piece{ PieceType::King, Color::White };

    squares_[7][0] = squares_[7][7] = Piece{ PieceType::Rook, Color::Black };
    squares_[7][1] = squares_[7][6] = Piece{ PieceType::Knight, Color::Black };
    squares_[7][2] = squares_[7][5] = Piece{ PieceType::Bishop, Color::Black };
    squares_[7][3] = Piece{ PieceType::Queen, Color::Black };
    squares_[7][4] = Piece{ PieceType::King, Color::Black };
}

void Board::clear_board() {
    for (auto& row : squares_) {
        row.fill(std::nullopt);
    }
}

void Board::set_piece(int rank, int file, PieceType type, Color color) {
    if (rank >= 0 && rank < Size && file >= 0 && file < Size) {
        squares_[rank][file] = Piece{type, color};
    }
}

const Board::Square& Board::at(int rank, int file) const {
    return squares_[rank][file];
}

Board::Square& Board::at(int rank, int file) {
    return squares_[rank][file];
}

std::string Board::to_string() const {
    std::string result;
    for (int rank = Size - 1; rank >= 0; --rank) {
        for (int file = 0; file < Size; ++file) {
            const auto& sq = squares_[rank][file];
            if (!sq) {
                result += ". ";
            } else {
                char c = '?';
                switch (sq->type) {
                    case PieceType::Pawn:   c = 'P'; break;
                    case PieceType::Knight: c = 'N'; break;
                    case PieceType::Bishop: c = 'B'; break;
                    case PieceType::Rook:   c = 'R'; break;
                    case PieceType::Queen:  c = 'Q'; break;
                    case PieceType::King:   c = 'K'; break;
                }
                if (sq->color == Color::Black)
                    c = std::tolower(c);
                result += std::format("{} ", c);
            }
        }
        result += "\n";
    }
    return result;
}

std::string Board::to_vt100_unicode_string() const {
    static const wchar_t* unicode_pieces[2][6] = {
        {L"♙", L"♘", L"♗", L"♖", L"♕", L"♔"},
        {L"♟", L"♞", L"♝", L"♜", L"♛", L"♚"}
    };
    static const char* bg_light = "\033[48;5;230m";
    static const char* bg_dark  = "\033[48;5;101m";
    static const char* reset    = "\033[0m";

    std::ostringstream oss;
    for (int rank = Size - 1; rank >= 0; --rank) {
        for (int file = 0; file < Size; ++file) {
            bool is_light = (rank + file) % 2 == 0;
            oss << (is_light ? bg_light : bg_dark);

            const auto& sq = squares_[rank][file];
            if (!sq) {
                oss << "  ";
            } else {
                int color_idx = (sq->color == Color::White) ? 0 : 1;
                int type_idx = static_cast<int>(sq->type);
                std::wstring ws(unicode_pieces[color_idx][type_idx]);
                std::string utf8(ws.begin(), ws.end());
                oss << utf8 << " ";
            }
            oss << reset;
        }
        oss << "\n";
    }
    return oss.str();
}

bool Board::set_fen(const std::string& fen) {
    clear_board();
    std::istringstream iss(fen);
    std::string board_part, active_color, castling, ep_square;
    if (!(iss >> board_part >> active_color >> castling >> ep_square)) return false;

    int rank = 7, file = 0;
    for (char c : board_part) {
        if (c == '/') {
            --rank;
            file = 0;
        }
        else if (std::isdigit(c)) {
            file += c - '0';
        }
        else {
            Color color = std::isupper(c) ? Color::White : Color::Black;
            PieceType type;
            switch (std::tolower(c)) {
            case 'p': type = PieceType::Pawn; break;
            case 'n': type = PieceType::Knight; break;
            case 'b': type = PieceType::Bishop; break;
            case 'r': type = PieceType::Rook; break;
            case 'q': type = PieceType::Queen; break;
            case 'k': type = PieceType::King; break;
            default: return false;
            }
            set_piece(rank, file, type, color);
            ++file;
        }
    }

    white_kingside_castle = false;
    white_queenside_castle = false;
    black_kingside_castle = false;
    black_queenside_castle = false;

    if (castling.find('K') != std::string::npos &&
        squares_[0][4] && squares_[0][4]->type == PieceType::King && squares_[0][4]->color == Color::White &&
        squares_[0][7] && squares_[0][7]->type == PieceType::Rook && squares_[0][7]->color == Color::White) {
        white_kingside_castle = true;
    }
    if (castling.find('Q') != std::string::npos &&
        squares_[0][4] && squares_[0][4]->type == PieceType::King && squares_[0][4]->color == Color::White &&
        squares_[0][0] && squares_[0][0]->type == PieceType::Rook && squares_[0][0]->color == Color::White) {
        white_queenside_castle = true;
    }
    if (castling.find('k') != std::string::npos &&
        squares_[7][4] && squares_[7][4]->type == PieceType::King && squares_[7][4]->color == Color::Black &&
        squares_[7][7] && squares_[7][7]->type == PieceType::Rook && squares_[7][7]->color == Color::Black) {
        black_kingside_castle = true;
    }
    if (castling.find('q') != std::string::npos &&
        squares_[7][4] && squares_[7][4]->type == PieceType::King && squares_[7][4]->color == Color::Black &&
        squares_[7][0] && squares_[7][0]->type == PieceType::Rook && squares_[7][0]->color == Color::Black) {
        black_queenside_castle = true;
    }

    if (ep_square == "-") {
        en_passant_target = std::nullopt;
    }
    else if (ep_square.size() == 2) {
        int file = ep_square[0] - 'a';
        int rank = ep_square[1] - '1';
        if (file >= 0 && file < Size && rank >= 0 && rank < Size) {
            en_passant_target = std::make_pair(rank, file);
        }
        else {
            en_passant_target = std::nullopt;
        }
    }

    return true;
}

void Board::update_castling_rights() {
    // For white kingside castling
    white_kingside_castle = (squares_[0][4] && squares_[0][4]->type == PieceType::King && squares_[0][4]->color == Color::White &&
                             squares_[0][7] && squares_[0][7]->type == PieceType::Rook && squares_[0][7]->color == Color::White);

    // For white queenside castling
    white_queenside_castle = (squares_[0][4] && squares_[0][4]->type == PieceType::King && squares_[0][4]->color == Color::White &&
                              squares_[0][0] && squares_[0][0]->type == PieceType::Rook && squares_[0][0]->color == Color::White);

    // For black kingside castling
    black_kingside_castle = (squares_[7][4] && squares_[7][4]->type == PieceType::King && squares_[7][4]->color == Color::Black &&
                             squares_[7][7] && squares_[7][7]->type == PieceType::Rook && squares_[7][7]->color == Color::Black);

    // For black queenside castling
    black_queenside_castle = (squares_[7][4] && squares_[7][4]->type == PieceType::King && squares_[7][4]->color == Color::Black &&
                              squares_[7][0] && squares_[7][0]->type == PieceType::Rook && squares_[7][0]->color == Color::Black);
}
