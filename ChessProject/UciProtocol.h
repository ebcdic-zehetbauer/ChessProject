#pragma once
#include "Board.h"
#include "Eval.h"
#include "Logger.h"
#include <string>
#include <atomic>
#include <thread>
#include <sstream>
#include <vector>

class UciProtocol {
public:
    explicit UciProtocol(Logger& logger);

    void run();

private:
    Logger& logger_;
    Board board_;
    MoveSelector move_selector_;
    std::atomic<bool> running_{true};

    void handle_command(const std::string& line);

    // UCI commands
    void cmd_uci();
    void cmd_isready();
    void cmd_ucinewgame();
    void cmd_position(const std::string& args);
    void cmd_go(const std::string& args);
    void cmd_quit();

    // TODO: Add advanced UCI commands (setoption, stop, ponderhit, etc.)
};