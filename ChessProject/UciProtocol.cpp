#include "UciProtocol.h"
#include <iostream>
#include <sstream>

UciProtocol::UciProtocol(Logger& logger)
    : logger_(logger), move_selector_(std::thread::hardware_concurrency()) {}

void UciProtocol::run() {
    logger_.log("UCI protocol run() started", LogLevel::Info);
    std::string line;
    while (running_ && std::getline(std::cin, line)) {
        handle_command(line);
    }
}

void UciProtocol::handle_command(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd == "uci") cmd_uci();
    else if (cmd == "isready") cmd_isready();
    else if (cmd == "ucinewgame") cmd_ucinewgame();
    else if (cmd == "position") cmd_position(line.substr(8));
    else if (cmd == "go") cmd_go(line.substr(2));
    else if (cmd == "quit") cmd_quit();
    // TODO: Add support for setoption, stop, ponderhit, etc.
    else if (cmd == "help") {
        std::cout << "Supported UCI commands: uci, isready, ucinewgame, position, go, quit" << std::endl;
        logger_.log("Handled help", LogLevel::Info);
    }
    else logger_.log("Unknown command: " + cmd, LogLevel::Warning);
}

void UciProtocol::cmd_uci() {
    std::cout << "id name MyChessEngine" << std::endl;
    std::cout << "id author YourName" << std::endl;
    std::cout << "uciok" << std::endl;
}

void UciProtocol::cmd_isready() {
    std::cout << "readyok" << std::endl;
}

void UciProtocol::cmd_ucinewgame() {
    board_.setup_initial_position();
    logger_.log("New game started (ucinewgame)", LogLevel::Info);
}

void UciProtocol::cmd_position(const std::string& args) {
    std::istringstream iss(args);
    std::string token;
    if (args.find("startpos") != std::string::npos) {
        board_.setup_initial_position();
    } else if (args.find("fen") != std::string::npos) {
        iss >> token; // skip "fen"
        std::string fen;
        std::getline(iss, fen);
        board_.set_fen(fen);
    }
    // TODO: Parse and apply moves after position
    logger_.log("Position set: " + args, LogLevel::Debug);
}

void UciProtocol::cmd_go(const std::string& args) {
    // TODO: Parse search parameters (depth, movetime, etc.)
    Color side = Color::White; // TODO: Determine from board state
    Move best = move_selector_.select_best_move(board_, side, 4);
    std::cout << "bestmove " << best.to_algebraic(board_) << std::endl;
    logger_.log("Best move sent: " + best.to_algebraic(board_), LogLevel::Info);
}

void UciProtocol::cmd_quit() {
    running_ = false;
    logger_.log("UCI protocol quitting", LogLevel::Info);
}