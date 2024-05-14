#include <memory>
#include <iostream>
#include <list>
#include <valarray>

typedef struct _Position
{
	int x = -1;
	int y = -1;

	float magnitude() const
	{
		return std::sqrt(static_cast<double>(x * x) + static_cast<double>(y * y));
	}
}Pos;

bool operator == (Pos a, Pos b)
{
	return a.x == b.x && a.y == b.y;
}
Pos operator - (Pos a, Pos b)
{
	return { a.x - b.x , a.y - b.y};
}

template <typename T>
bool contains(const std::list<T>& lst, const T& value) {
	return std::find(lst.begin(), lst.end(), value) != lst.end();
}

template <typename T>
void addUnique(std::list<T>& lst, const T& value) {
	if (!contains(lst, value)) {
		lst.push_back(value);
	} else {
		std::cout << "Element already exists in the list.\n";
	}
}

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
		status current = getStatus();
		s_ = s;
		return current == BLANK;
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
		TYPE_NEGA_SCOUT = 1
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

class AI_nega_scout : public AI {
private:
	static void order_moves(const std::list<Pos>& moves, std::list<Pos>& ordered_moves);
	int evaluate(int limit, int alpha, int beta, Board& board, const std::list<Pos>& blanks, Mass::status current, Pos& best_pos);
public:
	AI_nega_scout() {}
	~AI_nega_scout() {}

	bool think(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_NEGA_SCOUT:
		return new AI_nega_scout();
	default:
		return new AI_ordered();
		break;
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered;
	friend class AI_nega_scout;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum {
		BOARD_SIZE = 5,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

	std::list<Pos> blanks = std::list<Pos>();

public:
	Board() {
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() == Mass::BLANK) {
					blanks.push_back({x, y});
				}
			}
		}
	}
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
			if (y == BOARD_SIZE) { return(Board::WINNER) winner; }
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

	bool put(Pos pos, Mass::status mass) {
		if (pos.x < 0 || BOARD_SIZE <= pos.x ||
			pos.y < 0 || BOARD_SIZE <= pos.y) return false;// 盤面外
		if (mass_[pos.y][pos.x].put(mass))
		{
			blanks.remove(pos);
			return true;
		}
		addUnique(blanks, pos);
		return false;
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
//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "＋"; else
//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "＃"; else
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

	std::list<Pos> get_blanks() const
	{
		return blanks;
	}
};

bool AI_ordered::think(Board& b)
{
	std::list<Pos> list = b.get_blanks();
	if (list.empty()) return false;
	Pos point = list.front();
	return b.put(point, Mass::ENEMY);
}

void AI_nega_scout::order_moves(const std::list<Pos>& moves, std::list<Pos>& ordered_moves) {
	ordered_moves = moves;
	ordered_moves.sort([](const Pos& a, const Pos& b) {
		Pos center = { Board::BOARD_SIZE / 2, Board::BOARD_SIZE / 2 };
		return (a - center).magnitude() < (b - center).magnitude();
	});
}

int AI_nega_scout::evaluate(int limit, int alpha, int beta, Board& board, const std::list<Pos>& blanks, Mass::status current, Pos& best_pos)
{
	if (limit-- == 0) return 0;
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = board.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;
	
	int a = alpha;
	int b = beta;

	std::list<Pos> ordered_moves;
	order_moves(blanks, ordered_moves);

	for (Pos blank : ordered_moves)
	{
		board.put(blank, current);
		std::list<Pos> current_list = board.get_blanks();
		Pos dummy;
		int score = -evaluate(limit, -b, -a, board, current_list, next, dummy);
		if (a < score && score < beta && !(blank.x == 0 && blank.y == 0) && limit <= 2)
		{
			a = -evaluate(limit, -beta, -score, board, current_list, next, dummy);
		}
		board.put(blank, Mass::BLANK);
		if (a < score)
		{
			a = score;
			best_pos = blank;
		}

		if (beta <= a)
		{
			return a;
		}
		b = a + 1;
	}
	return a;
}


bool AI_nega_scout::think(Board& b)
{

	Pos point;

	if (evaluate(5, -10000, 10000, b, b.get_blanks(), Mass::ENEMY, point) <= -9999)
	{
		return false;
	}
	return b.put(point, Mass::ENEMY);
}



class Game
{
private:
	const AI::type ai_type = AI::TYPE_NEGA_SCOUT;

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISED;
	AI* pAI_ = nullptr;

public:
	Game() {
		pAI_ = AI::createAi(ai_type);
	}
	~Game() {
		delete pAI_;
	}

	bool put(int x, int y) {
		bool success = board_.put({x, y}, Mass::PLAYER);
		if (success) winner_ = board_.calc_result();

		return success;
	}

	bool think() {
		bool success = pAI_->think(board_);
		std::cout << success << std::endl;
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
	for (;;) {// 無限ループ
		show_start_message();

		// initialize
		unsigned int turn = 0;
		std::shared_ptr<Game> game(new Game());

		while (1) {
			game->show();// 盤面表示

			// 勝利判定
			Board::WINNER winner = game->is_finised();
			if (winner) {
				show_end_message(winner);
				break;
			}

			if (0 == turn) {
				// user input
				char col[1], row[1];
				do {
					std::cout << "? ";
					std::cin >> row >> col;
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				// AI
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);// 投了
				}
				std::cout << std::endl;
			}
			// プレイヤーとAIの切り替え
			turn = 1 - turn;
		}
	}

	return 0;
}