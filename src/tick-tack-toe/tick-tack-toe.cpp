#include <memory>
#include <iostream>

class Mass {
public:
    enum class status {
        BLANK,
        PLAYER,
        ENEMY,
    };
private:
    status s_ = status::BLANK;
public:
    void setStatus(status s) { s_ = s; }
    status getStatus() const { return s_; }

    bool put(status s) {
        if (s_ != status::BLANK) return false;
        s_ = s;
        return true;
    }
};

class Board;

class AI {
public:
    AI() {}
    virtual ~AI() {}

    virtual bool think(Board& b) = 0;

public:
    enum class type {
        TYPE_ORDERED = 0,
        TYPE_MINIMAX,
    };

    static AI* createAi(type type);
};

// 順番に打ってみる
class AI_ordered : public AI {
public:
    AI_ordered() {}
    ~AI_ordered() {}

    bool think(Board& b);
};

class AI_minimax : public AI {
public:
    AI_minimax() {}
    ~AI_minimax() {}

    bool think(Board& b);
private:
    int minimax(Board& b, int depth, bool isMax);
    int evaluate(Board& b);
    bool isMovesLeft(Board& b);
};

AI* AI::createAi(type type)
{
    switch (type) {
    case type::TYPE_ORDERED:
        return new AI_ordered();
    case type::TYPE_MINIMAX:
        return new AI_minimax();
    default:
        return nullptr;
    }
}

class Board
{
    friend class AI_ordered;
    friend class AI_minimax;

public:
    enum class WINNER {
        NOT_FINISHED = 0,
        PLAYER,
        ENEMY,
        DRAW,
    };
private:
    static const int BOARD_SIZE = 3;
    Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
    Board() {}

    WINNER calc_result() const
    {
        // 縦横斜めに同じキャラが入っているか検索
        // 横
        for (int y = 0; y < BOARD_SIZE; y++) {
            Mass::status winner = mass_[y][0].getStatus();
            if (winner != Mass::status::PLAYER && winner != Mass::status::ENEMY) continue;
            int x = 1;
            for (; x < BOARD_SIZE; x++) {
                if (mass_[y][x].getStatus() != winner) break;
            }
            if (x == BOARD_SIZE) { return static_cast<WINNER>(winner); }
        }
        // 縦
        for (int x = 0; x < BOARD_SIZE; x++) {
            Mass::status winner = mass_[0][x].getStatus();
            if (winner != Mass::status::PLAYER && winner != Mass::status::ENEMY) continue;
            int y = 1;
            for (; y < BOARD_SIZE; y++) {
                if (mass_[y][x].getStatus() != winner) break;
            }
            if (y == BOARD_SIZE) { return static_cast<WINNER>(winner); }
        }
        // 斜め
        {
            Mass::status winner = mass_[0][0].getStatus();
            if (winner == Mass::status::PLAYER || winner == Mass::status::ENEMY) {
                int idx = 1;
                for (; idx < BOARD_SIZE; idx++) {
                    if (mass_[idx][idx].getStatus() != winner) break;
                }
                if (idx == BOARD_SIZE) { return static_cast<WINNER>(winner); }
            }
        }
        {
            Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
            if (winner == Mass::status::PLAYER || winner == Mass::status::ENEMY) {
                int idx = 1;
                for (; idx < BOARD_SIZE; idx++) {
                    if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
                }
                if (idx == BOARD_SIZE) { return static_cast<WINNER>(winner); }
            }
        }
        // 上記勝敗がついておらず、空いているマスがなければ引分け
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                Mass::status fill = mass_[y][x].getStatus();
                if (fill == Mass::status::BLANK) return WINNER::NOT_FINISHED;
            }
        }
        return WINNER::DRAW;
    }

    bool put(int x, int y) {
        if (x < 0 || BOARD_SIZE <= x ||
            y < 0 || BOARD_SIZE <= y) return false; // 盤面外
        return mass_[y][x].put(Mass::status::PLAYER);
    }

    void show() const {
        std::cout << "　　";
        for (int x = 0; x < BOARD_SIZE; x++) {
            std::cout << " " << x + 1 << "　";
        }
        std::cout << "\n　";
        for (int x = 0; x < BOARD_SIZE; x++) {
            std::cout << "＋－";
        }
        std::cout << "＋\n";
        for (int y = 0; y < BOARD_SIZE; y++) {
            std::cout << " " << char('a' + y);
            for (int x = 0; x < BOARD_SIZE; x++) {
                std::cout << "｜";
                switch (mass_[y][x].getStatus()) {
                case Mass::status::PLAYER:
                    std::cout << "〇";
                    break;
                case Mass::status::ENEMY:
                    std::cout << "×";
                    break;
                case Mass::status::BLANK:
                    std::cout << "　";
                    break;
                default:
                    std::cout << "　";
                }
            }
            std::cout << "｜\n";
            std::cout << "　";
            for (int x = 0; x < BOARD_SIZE; x++) {
                std::cout << "＋－";
            }
            std::cout << "＋\n";
        }
    }
};

bool AI_ordered::think(Board& b)
{
    for (int y = 0; y < Board::BOARD_SIZE; y++) {
        for (int x = 0; x < Board::BOARD_SIZE; x++) {
            if (b.mass_[y][x].put(Mass::status::ENEMY)) {
                return true;
            }
        }
    }
    return false;
}

bool AI_minimax::think(Board& b) {
    int bestVal = -1000;
    int bestMoveRow = -1;
    int bestMoveCol = -1;

    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        for (int j = 0; j < Board::BOARD_SIZE; j++) {
            if (b.mass_[i][j].getStatus() == Mass::status::BLANK) {
                b.mass_[i][j].setStatus(Mass::status::ENEMY);
                int moveVal = minimax(b, 0, false);
                b.mass_[i][j].setStatus(Mass::status::BLANK);
                if (moveVal > bestVal) {
                    bestMoveRow = i;
                    bestMoveCol = j;
                    bestVal = moveVal;
                }
            }
        }
    }
    b.mass_[bestMoveRow][bestMoveCol].setStatus(Mass::status::ENEMY);
    return true;
}

int AI_minimax::minimax(Board& b, int depth, bool isMax) {
    int score = evaluate(b);

    if (score == 10)
        return score - depth;
    if (score == -10)
        return score + depth;
    if (!isMovesLeft(b))
        return 0;

    if (isMax) {
        int best = -1000;
        for (int i = 0; i < Board::BOARD_SIZE; i++) {
            for (int j = 0; j < Board::BOARD_SIZE; j++) {
                if (b.mass_[i][j].getStatus() == Mass::status::BLANK) {
                    b.mass_[i][j].setStatus(Mass::status::ENEMY);
                    best = std::max(best, minimax(b, depth + 1, !isMax));
                    b.mass_[i][j].setStatus(Mass::status::BLANK);
                }
            }
        }
        return best;
    }
    else {
        int best = 1000;
        for (int i = 0; i < Board::BOARD_SIZE; i++) {
            for (int j = 0; j < Board::BOARD_SIZE; j++) {
                if (b.mass_[i][j].getStatus() == Mass::status::BLANK) {
                    b.mass_[i][j].setStatus(Mass::status::PLAYER);
                    best = std::min(best, minimax(b, depth + 1, !isMax));
                    b.mass_[i][j].setStatus(Mass::status::BLANK);
                }
            }
        }
        return best;
    }
}

int AI_minimax::evaluate(Board& b) {
    // プレイヤーが勝った場合
    if (b.calc_result() == Board::WINNER::PLAYER)
        return -10;
    // 敵が勝った場合
    if (b.calc_result() == Board::WINNER::ENEMY)
        return 10;
    // その他の場合
    return 0;
}

bool AI_minimax::isMovesLeft(Board& b) {
    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        for (int j = 0; j < Board::BOARD_SIZE; j++) {
            if (b.mass_[i][j].getStatus() == Mass::status::BLANK)
                return true;
        }
    }
    return false;
}

class Game
{
private:
    const AI::type ai_type = AI::type::TYPE_MINIMAX;

    Board board_;
    Board::WINNER winner_ = Board::WINNER::NOT_FINISHED;
    AI* pAI_ = nullptr;

public:
    Game() {
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

void show_start_message()
{
    std::cout << "========================" << std::endl;
    std::cout << "       GAME START       " << std::endl;
    std::cout << std::endl;
    std::cout << "input position like 1 a" << std::endl;
    std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
    if (winner == Board::WINNER::PLAYER) {
        std::cout << "You win!" << std::endl;
    }
    else if (winner == Board::WINNER::ENEMY)
    {
        std::cout << "You lose..." << std::endl;
    }
    else {
        std::cout << "Draw" << std::endl;
    }
    std::cout << std::endl;
}

int main()
{
    for (;;) {// 無限ループ
        show_start_message();

        // initialize
        unsigned int turn = 0;
        std::shared_ptr<Game> game(new Game());

        while (1) {
            game->show();// 盤面表示

            // 勝利判定
            Board::WINNER winner = game->is_finished();
            if (winner != Board::WINNER::NOT_FINISHED) {
                show_end_message(winner);
                break;
            }

            if (0 == turn) {
                // user input
                char col, row;
                do {
                    std::cout << "? ";
                    std::cin >> row >> col;
                } while (!game->put(row - '1', col - 'a'));
            }
            else {
                // AI
                if (!game->think()) {
                    show_end_message(Board::WINNER::PLAYER);// 投了
                    break;
                }
                std::cout << std::endl;
            }
            // プレイヤーとAIの切り替え
            turn = 1 - turn;
        }
    }

    return 0;
}
