#include <print>
#include "Board.h"
#include "Move.h"
#include "MoveGen.h"
#include "Eval.h"

// Demo: Let the engine play moves for both sides
void play_demo_game(Board& board, Color side_to_move = Color::Black) {
    int ply = 0;
    while (true) {
        auto result = generate_legal_moves(&board, side_to_move);
        if (!result || result->empty()) {
            // Check for checkmate or stalemate
            if (king_in_check(board, side_to_move)) {
                std::println("Checkmate! {} wins.", side_to_move == Color::White ? "Black" : "White");
            }
            else {
                std::println("Stalemate! Draw.");
            }
            break;
        }
        Move best = select_best_move(board, side_to_move, 2);
        std::println("Ply {}: {} plays {}", ++ply, side_to_move == Color::White ? "White" : "Black", best.to_algebraic(board));
        apply_move(board, best);
        std::println("{}", board.to_string());
        side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;
    }
}

int main() {
    Board board; // Ensure an object of Board is created
    std::println("{}", board.to_string());

    board.clear_board(); // Call the non-static member function on the object
    std::println("{}", board.to_string());

	board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	std::println("{}", board.to_string());

    board.set_fen("8/8/8/8/8/8/4K3/4k3");
	std::println("{}", board.to_string());

    // using https://www.redhotpawn.com/chess/chess-fen-viewer.php to verify the moves.

    board.set_fen("r2q1r2/ppp2pbk/4b1pp/4p3/4P3/3P4/PPP3BP/R1Q1R1K1 w - - 0 1");    // From: https://chessfox.com/chess-puzzles-for-beginners/
	std::println("{}", board.to_string());

	// std::println("Generating legal moves for White...");
    auto result = generate_legal_moves(&board, Color::White);

    if (result && !result->empty()) {
        // for (const auto& move : *result) { std::println("{}", move.to_algebraic(board)); }
        // Find and print the best move for White using minimax (2-ply)
        Move best = select_best_move(board, Color::White, 2);
        std::println("Best move for White (2-ply minimax): {}", best.to_algebraic(board));
    } else {
        std::println("Error: {}", result ? "No legal moves" : result.error());
    }

	// std::println("Generating legal moves for Black...");
	result = generate_legal_moves(&board, Color::Black);
    if (result) {
        // for (const auto& move : *result) { std::println("{}", move.to_algebraic(board)); }
		// Find and print the best move for Black using minimax (2-ply)
        Move best = select_best_move(board, Color::Black, 2);
		std::println("Best move for Black (2-ply minimax): {}", best.to_algebraic(board));

        apply_move(board, best); // Apply the best move found
        std::println("{}", board.to_string());
    } else {
        std::println("Error: {}", result ? "No legal moves" : result.error());
	}

	// Play a demo game with the engine playing both sides
    // play_demo_game(board, Color::Black);

    return 0;
}