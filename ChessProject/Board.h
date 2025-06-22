#pragma once
#include <array>
#include <optional>
#include <string>
#include <compare>

enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };
enum class Color { White, Black };

struct Piece {
    PieceType type;
    Color color;

    auto operator<=>(const Piece&) const = default;
};

class Board {
public:
    static constexpr int Size = 8;
    using Square = std::optional<Piece>;
    using BoardArray = std::array<std::array<Square, Size>, Size>;

    Board();

    // Accessors
    const Square& at(int rank, int file) const;
    Square& at(int rank, int file);

    // Utility
    std::string to_string() const;
    std::string to_vt100_unicode_string() const;

    // User piece placement
    void clear_board();
    void set_piece(int rank, int file, PieceType type, Color color);

    // FEN support
    bool set_fen(const std::string& fen);

    const std::optional<std::pair<int, int>>& get_en_passant_target() const { return en_passant_target; }
    void set_en_passant_target(const std::optional<std::pair<int, int>>& ep) { en_passant_target = ep; }

    void update_castling_rights();

    bool white_kingside_castle = true;
    bool white_queenside_castle = true;
    bool black_kingside_castle = true;
    bool black_queenside_castle = true;

private:
    BoardArray squares_;
    std::optional<std::pair<int, int>> en_passant_target;
    void setup_initial_position();
};