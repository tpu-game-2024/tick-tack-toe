#include <memory>
#include <iostream>
#include <limits>
#include <vector>

class Mass {
public:
    enum status {
        BLANK,
        PLAYER,
        ENEMY,
    };
private:
    status s_ = BLANK;
public:
    void setStatus(status s) { s_ = s; }
    status getStatus() const { return s_; }

    bool put(status s) {
        if (s_ != BLANK) return false;
        s_ = s;
        return true;
    }
};

class Board;

class AI_alphaBeta;

class AI {
public:
    AI() {}
    virtual ~AI() {}

    virtual bool think(Board& b) = 0;

public:
    enum type {
        TYPE_ORDERED = 0,
        TYPE_ALPHA_BETA,
    };

    static AI* createAi(type type);
};

class AI_ordered : public AI {
public:
    AI_ordered() {}
    ~AI_ordered() {}

    bool think(Board& b);
};

class Board {
    friend class AI_ordered;
    friend class AI_alphaBeta;

public:
    enum WINNER {
        NOT_FINISED = 0,
        PLAYER,
        ENEMY,
        DRAW,
    };
private:
    int board_size_;
    std::vector<std::vector<Mass>> mass_;

public:
    Board(int size) : board_size_(size), mass_(size, std::vector<Mass>(size)) {}

    Board::WINNER calc_result() const {
        for (int y = 0; y < board_size_; y++) {
            Mass::status winner = mass_[y][0].getStatus();
            if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
            int x = 1;
            for (; x < board_size_; x++) {
                if (mass_[y][x].getStatus() != winner) break;
            }
            if (x == board_size_) { return (Board::WINNER)winner; }
        }
        for (int x = 0; x < board_size_; x++) {
            Mass::status winner = mass_[0][x].getStatus();
            if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
            int y = 1;
            for (; y < board_size_; y++) {
                if (mass_[y][x].getStatus() != winner) break;
            }
            if (y == board_size_) { return(Board::WINNER) winner; }
        }
        {
            Mass::status winner = mass_[0][0].getStatus();
            if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
                int idx = 1;
                for (; idx < board_size_; idx++) {
                    if (mass_[idx][idx].getStatus() != winner) break;
                }
                if (idx == board_size_) { return (Board::WINNER)winner; }
            }
        }
        {
            Mass::status winner = mass_[board_size_ - 1][0].getStatus();
            if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
                int idx = 1;
                for (; idx < board_size_; idx++) {
                    if (mass_[board_size_ - 1 - idx][idx].getStatus() != winner) break;
                }
                if (idx == board_size_) { return (Board::WINNER)winner; }
            }
        }
        for (int y = 0; y < board_size_; y++) {
            for (int x = 0; x < board_size_; x++) {
                Mass::status fill = mass_[y][x].getStatus();
                if (fill == Mass::BLANK) return NOT_FINISED;
            }
        }
        return DRAW;
    }

    bool put(int x, int y, Mass::status s) {
        if (x < 0 || board_size_ <= x ||
            y < 0 || board_size_ <= y) return false;
        return mass_[y][x].put(s);
    }

    bool put(int x, int y) {
        return put(x, y, Mass::PLAYER);
    }

    void undo(int x, int y) {
        mass_[y][x].setStatus(Mass::BLANK);
    }

    void show() const {
        std::cout << "   ";
        for (int x = 0; x < board_size_; x++) {
            std::cout << " " << x + 1 << " ";
        }
        std::cout << "\n  ";
        for (int x = 0; x < board_size_; x++) {
            std::cout << "+-";
        }
        std::cout << "+\n";
        for (int y = 0; y < board_size_; y++) {
            std::cout << " " << char('a' + y);
            for (int x = 0; x < board_size_; x++) {
                std::cout << "|";
                switch (mass_[y][x].getStatus()) {
                case Mass::PLAYER:
                    std::cout << "O";
                    break;
                case Mass::ENEMY:
                    std::cout << "X";
                    break;
                case Mass::BLANK:
                    std::cout << " ";
                    break;
                default:
                    std::cout << " ";
                }
            }
            std::cout << "|\n  ";
            for (int x = 0; x < board_size_; x++) {
                std::cout << "+-";
            }
            std::cout << "+\n";
        }
    }
};

bool AI_ordered::think(Board& b) {
    for (int y = 0; y < b.board_size_; y++) {
        for (int x = 0; x < b.board_size_; x++) {
            if (b.mass_[y][x].put(Mass::ENEMY)) {
                return true;
            }
        }
    }
    return false;
}

class AI_alphaBeta : public AI {
public:
    AI_alphaBeta() {}
    ~AI_alphaBeta() {}

    bool think(Board& b);
private:
    int alphaBeta(Board& b, int depth, int alpha, int beta, bool maximizingPlayer);
    int evaluate(const Board& b);
    static const int MAX_DEPTH = 5;  // Maximum depth for the search
};

bool AI_alphaBeta::think(Board& b) {
    int bestValue = std::numeric_limits<int>::min();
    int bestMoveX = -1;
    int bestMoveY = -1;

    for (int y = 0; y < b.board_size_; y++) {
        for (int x = 0; x < b.board_size_; x++) {
            if (b.mass_[y][x].getStatus() == Mass::BLANK) {
                b.mass_[y][x].setStatus(Mass::ENEMY);
                int moveValue = alphaBeta(b, 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
                b.undo(x, y);
                if (moveValue > bestValue) {
                    bestMoveX = x;
                    bestMoveY = y;
                    bestValue = moveValue;
                }
            }
        }
    }
    if (bestMoveX != -1 && bestMoveY != -1) {
        return b.put(bestMoveX, bestMoveY, Mass::ENEMY);
    }
    return false;
}

int AI_alphaBeta::alphaBeta(Board& b, int depth, int alpha, int beta, bool maximizingPlayer) {
    Board::WINNER result = b.calc_result();
    if (result != Board::NOT_FINISED || depth == MAX_DEPTH) {
        return evaluate(b);
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (int y = 0; y < b.board_size_; y++) {
            for (int x = 0; x < b.board_size_; x++) {
                if (b.mass_[y][x].getStatus() == Mass::BLANK) {
                    b.mass_[y][x].setStatus(Mass::ENEMY);
                    int eval = alphaBeta(b, depth + 1, alpha, beta, false);
                    b.undo(x, y);
                    maxEval = std::max(maxEval, eval);
                    alpha = std::max(alpha, eval);
                    if (beta <= alpha) {
                        return maxEval;
                    }
                }
            }
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (int y = 0; y < b.board_size_; y++) {
            for (int x = 0; x < b.board_size_; x++) {
                if (b.mass_[y][x].getStatus() == Mass::BLANK) {
                    b.mass_[y][x].setStatus(Mass::PLAYER);
                    int eval = alphaBeta(b, depth + 1, alpha, beta, true);
                    b.undo(x, y);
                    minEval = std::min(minEval, eval);
                    beta = std::min(beta, eval);
                    if (beta <= alpha) {
                        return minEval;
                    }
                }
            }
        }
        return minEval;
    }
}

int AI_alphaBeta::evaluate(const Board& b) {
    Board::WINNER result = b.calc_result();
    if (result == Board::ENEMY) {
        return 10;
    } else if (result == Board::PLAYER) {
        return -10;
    } else {
        return 0;
    }
}

AI* AI::createAi(type type) {
    switch (type) {
        case TYPE_ORDERED:
            return new AI_ordered();
        case TYPE_ALPHA_BETA:
            return new AI_alphaBeta();
        default:
            return nullptr;
    }
}

class Game {
private:
    const AI::type ai_type = AI::TYPE_ALPHA_BETA;

    Board board_;
    Board::WINNER winner_ = Board::NOT_FINISED;
    AI* pAI_ = nullptr;

public:
    Game(int board_size) : board_(board_size) {
        pAI_ = AI::createAi(ai_type);
    }
    ~Game() {
        delete pAI_;
    }

    bool put(int x, int y) {
        bool success = board_.put(x, y);
        if (success) winner_ = board_.calc_result();
        return success;
    }

    bool think() {
        bool success = pAI_->think(board_);
        if (success) winner_ = board_.calc_result();
        return success;
    }

    Board::WINNER is_finished() {
        return winner_;
    }

    void show() {
        board_.show();
    }
};

void show_start_message() {
    std::cout << "========================" << std::endl;
    std::cout << "       GAME START       " << std::endl;
    std::cout << std::endl;
    std::cout << "input position like 1 a" << std::endl;
    std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner) {
    if (winner == Board::PLAYER) {
        std::cout << "You win!" << std::endl;
    } else if (winner == Board::ENEMY) {
        std::cout << "You lose..." << std::endl;
    } else {
        std::cout << "Draw" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    int board_size;
    std::cout << "Enter board size: ";
    std::cin >> board_size;

    for (;;) {
        show_start_message();

        unsigned int turn = 0;
        std::shared_ptr<Game> game(new Game(board_size));

        while (1) {
            game->show();

            Board::WINNER winner = game->is_finished();
            if (winner != Board::NOT_FINISED) {
                show_end_message(winner);
                break;
            }

            if (0 == turn) {
                char col, row;
                do {
                    std::cout << "? ";
                    std::cin >> row >> col;
                } while (!game->put(row - '1', col - 'a'));
            } else {
                if (!game->think()) {
                    show_end_message(Board::WINNER::PLAYER);
                }
                std::cout << std::endl;
            }
            turn = 1 - turn;
        }
    }

    return 0;
}
