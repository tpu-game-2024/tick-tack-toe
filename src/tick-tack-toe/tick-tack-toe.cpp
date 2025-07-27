#include <memory>
#include <iostream>

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

class AI {
public:
	AI() {}
	virtual ~AI() {}

	virtual bool think(Board& b) = 0;

public:
	enum type {
		TYPE_ORDERED = 0,
		TYPE_SMART = 1,
	};

	static AI* createAi(type type);
};

class AI_ordered : public AI {
public:
	AI_ordered() {}
	~AI_ordered() {}

	bool think(Board& b);
};

class AI_smart : public AI {
public:
	AI_smart() {}
	~AI_smart() {}

	bool think(Board& b);

private:
	bool win(Board& b, int& x, int& y);
	bool guard(Board& b, int& x, int& y);
	bool center(Board& b);
	bool empty(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_SMART:
		return new AI_smart();
	case TYPE_ORDERED:
	default:
		return new AI_ordered();
	}
}

class Board
{
	friend class AI_ordered;
	friend class AI_smart;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum {
		BOARD_SIZE = 3,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {}

	Board::WINNER calc_result() const
	{
		// 縦横斜めに同じキャラが入っているか検索
		// 横
		for (int y = 0; y < BOARD_SIZE; y++) {
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER)winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISED;
			}
		}
		return DRAW;
	}

	bool put(int x, int y) {
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		return mass_[y][x].put(Mass::PLAYER);
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
				case Mass::PLAYER:
					std::cout << "〇";
					break;
				case Mass::ENEMY:
					std::cout << "×";
					break;
				case Mass::BLANK:
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
			if (b.mass_[y][x].put(Mass::ENEMY)) {
				return true;
			}
		}
	}
	return false;
}

bool AI_smart::think(Board& b)
{
	int x, y;

	if (win(b, x, y)) {
		return b.mass_[y][x].put(Mass::ENEMY);
	}

	if (guard(b, x, y)) {
		return b.mass_[y][x].put(Mass::ENEMY);
	}

	if (center(b)) {
		return true;
	}
	return empty(b);
}

bool AI_smart::win(Board& b, int& winX, int& winY)
{
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			if (b.mass_[y][x].getStatus() == Mass::BLANK) {
				b.mass_[y][x].setStatus(Mass::ENEMY);

				Board::WINNER result = b.calc_result();

				b.mass_[y][x].setStatus(Mass::BLANK);

				if (result == Board::ENEMY) {
					winX = x;
					winY = y;
					return true;
				}
			}
		}
	}
	return false;
}

bool AI_smart::guard(Board& b, int& blockX, int& blockY)
{
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			if (b.mass_[y][x].getStatus() == Mass::BLANK) {
				b.mass_[y][x].setStatus(Mass::PLAYER);

				Board::WINNER result = b.calc_result();

				b.mass_[y][x].setStatus(Mass::BLANK);

				if (result == Board::PLAYER) {
					blockX = x;
					blockY = y;
					return true;
				}
			}
		}
	}
	return false;
}

bool AI_smart::center(Board& b)
{
	if (b.mass_[1][1].getStatus() == Mass::BLANK) {
		return b.mass_[1][1].put(Mass::ENEMY);
	}
	return false;
}

bool AI_smart::empty(Board& b)
{
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			if (b.mass_[y][x].put(Mass::ENEMY)) {
				return true;
			}
		}
	}
	return false;
}

class Game
{
private:
	AI::type ai_type;

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISED;
	AI* pAI_ = nullptr;

public:
	Game(AI::type type = AI::TYPE_SMART) : ai_type(type) {
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

	Board::WINNER is_finised() {
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
	std::cout << "input position likes 1 a" << std::endl;
	std::cout << "AI 難易度:" << std::endl;
	std::cout << "0: 簡単" << std::endl;
	std::cout << "1: 一般）" << std::endl;
	std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
	if (winner == Board::PLAYER) {
		std::cout << "You win!" << std::endl;
	}
	else if (winner == Board::ENEMY)
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
	for (;;) {
		show_start_message();

		int aiLevel;
		std::cout << "AI難易度 (0-1): ";
		std::cin >> aiLevel;
		if (aiLevel < 0 || aiLevel > 1) aiLevel = 1;

		unsigned int turn = 0;
		std::shared_ptr<Game> game(new Game((AI::type)aiLevel));

		while (1) {
			game->show();

			Board::WINNER winner = game->is_finised();
			if (winner) {
				show_end_message(winner);
				break;
			}

			if (0 == turn) {
				char col[1], row[1];
				do {
					std::cout << "? ";
					std::cin >> row >> col;
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);
					break;
				}
				std::cout << std::endl;
			}
			turn = 1 - turn;
		}
	}

	return 0;
}