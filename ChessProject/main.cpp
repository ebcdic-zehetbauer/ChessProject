#include <string>
#include <print>
#include <algorithm>
#include <execution>
#include <vector>
#include <tuple>
#include <future>
#include "Board.h"
#include "Move.h"
#include "MoveGen.h"
#include "Eval.h"
#include "UciProtocol.h"
#include "Logger.h"
//#include "TuiApp.h"
//#include <notcurses/notcurses.h>

const static std::string vt100_RED = "\033[31m";    // set text color to red
const static std::string vt100_GREEN = "\033[32m";  // set text color to green
const static std::string vt100_RESET = "\033[0m";   // reset text color
const static std::string vt100_BOLD = "\033[1m";    // set text to bold

void vt100_move_cursor(int row, int col) {
    std::printf("\033[%d;%dH", row, col); // Move cursor to (row, col)
}

// Demo: Let the engine play moves for both sides
static void play_demo_game(Board& board, Color side_to_move = Color::Black) {
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
        // Move best = select_best_move(board, side_to_move, 2);
        MoveSelector selector;
        Move best = selector.select_best_move(board, side_to_move, 2);

        std::println("Ply {}: {} plays {}", ++ply, side_to_move == Color::White ? "White" : "Black", best.to_algebraic(board));
        apply_move(board, best);
        std::println("{}", board.to_string());
        side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;
    }
}

static void test_tui() {
    // This function is for testing the TUI (Text User Interface) functionality.
    // You can implement your TUI logic here.
    std::println("TUI test function called.");

    Board board; // Ensure an object of Board is created
    std::println("Default board\n{}", board.to_string());
	std::println("Board in unicode\n{}", board.to_vt100_unicode_string());

    board.clear_board(); // Call the non-static member function on the object
    std::println("Clearing the board\n{}", board.to_string());

    board.set_fen("8/8/8/8/8/8/4K3/4k3");
    std::println("Setting board up with custom fen string.\n{}", board.to_string());

    board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::println("Board defined with set_fin\n{}", board.to_string());
    std::println("Board in unicode\n{}", board.to_vt100_unicode_string());
}

static bool test_scenario(std::string fen, std::string answer, Color c) {
    // This function is a placeholder for testing specific scenarios.
    // You can implement your scenario logic here.
    std::println("Scenario test function called.");
    Board board; // Ensure an object of Board is created

    board.set_fen(fen);
    std::println("Setting up board with custom fen string.\n{}", board.to_string());

	MoveSelector selector; // Create a MoveSelector instance
	Move best = selector.select_best_move(board, c, 5); // needs to be at least 5-ply for the intermediate scenarios to work
                                                        // 5 is also a good balance for now.
    std::println("Best move for {}: {}", c == Color::White ? "White" : "Black", best.to_algebraic(board));
    apply_move(board, best); // Apply the best move found

    std::println("Resulting board.\n{}", board.to_string());

    return (answer == best.to_algebraic(board));
}

static void testing_the_scenarios() {


    bool test_tally = true;     // assume everything will pass LOL

    // using https://www.redhotpawn.com/chess/chess-fen-viewer.php to verify the moves.
    std::vector<std::tuple<std::string, std::string, Color>> scenario = {
		{"r2q1r2/ppp2pbk/4b1pp/4p3/4P3/3P4/PPP3BP/R1Q1R1K1 w - - 0 1", "d8d4", Color::Black},       // easy chess puzzle #1 (chessfox)
		{"r3k3/1p3p2/2p1qP1Q/p7/3p4/P2P1B2/1PP2KPP/8 w - -0 1", "h6h8", Color::White},              //                   #2 (chessfox)
        {"1KR4R/1PPB4/PQN4P/5PP1/8/p2b1pbp/1pp2qp1/1k1rr3 w - - 0 1", "f2b6", Color::Black},        //                   #3
		{"r5k1/1R3bp1/3p3p/2q2p2/p1P1pP2/P3P1P1/1Q1N1K1P/8 b - - 0 1", "b7f7", Color::White}        // intermediate chess puzzle #1
     };

	// Loop through each scenario and test it
    for (const auto& [fen, answer, color] : scenario) {
		vt100_move_cursor(0, 0); // Move cursor to the top left corner of the terminal
        std::println("Testing scenario with FEN: {}", fen);
        if (test_scenario(fen, answer, color)) {
            std::println("Test {}passed{} for FEN.", vt100_GREEN, vt100_RESET);
        } else {
            std::println("Test {}failed{} for FEN.", vt100_RED, vt100_RESET);
            test_tally = false; // If any test fails, set the tally to false
        }
	}
}

int main(int argc, char* argv[]) {
    Board board; // Ensure an object of Board is created

	Logger logger; // Create a Logger instance
	UciProtocol uci(logger); // Create a UCI protocol instance

    uci.run();

    std::println("Default board\n{}", board.to_string());

    test_tui(); // Test the TUI functionality

    testing_the_scenarios();

    std::println("Running demo game with this board.\n{}", board.to_string());
    //play_demo_game(board, Color::Black);

    std::println("Demo game finished. Final board state:\n{}", board.to_string());
    std::println("Testing the TUI application...");

    /*
    Logger logger;
    UciProtocol uci(logger);

    uci.run();
    */

 /*
    if (argc > 1 && std::string(argv[1]) == "--uci") {
        Logger logger;
        UciProtocol uci(logger);
        uci.run();
    }
    else {
        TuiApp app;
        app.run();
    }
    */
        return 0;
}