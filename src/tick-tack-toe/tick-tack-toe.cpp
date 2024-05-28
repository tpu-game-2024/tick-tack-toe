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
		NEGA_MAX = 1,
		ALPHA_BETA = 2,
		NEGA_SCOUT = 3,
		MONTE_CARLO = 4,
		MONTECARLO_TREE = 5,
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

class AI_nega_max : public AI {
private:
	int evaluate(Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_nega_max() {}
	~AI_nega_max() {}

	bool think(Board& b);
};

class AI_alpha_beta : public AI {
private:
	int evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_alpha_beta() {}
	~AI_alpha_beta() {}

	bool think(Board& b);
};

class AI_nega_scout : public AI {
private:
	int evalueate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y);
public:
	AI_nega_scout() {}
	~AI_nega_scout() {}

	bool think(Board& b);
};

class AI_monte_carlo : public AI {
private:
	int evaluate(bool first_time, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_monte_carlo() {}
	~AI_monte_carlo() {}

	bool think(Board& b);
};

class AI_montecarlo_tree : public AI {
private:
	static int select_mass(int n, int* a_count, int* a_wins);
	int evaluate(bool all_search, int count, Board& b, Mass::status current, int& best_x, int &best_y);
public:
	AI_montecarlo_tree() {}
	~AI_montecarlo_tree() {}

	bool will_win(Board &b, int& best_x, int& best_y);
	bool will_lose(Board &b, int &best_x, int& best_y);

	bool think(Board& b);
};
 
AI* AI::createAi(type type)
{
	switch (type) {
		case TYPE_ORDERED:
			return new AI_ordered();
			break;
		case NEGA_MAX:
			return new AI_nega_max();
			break;
		case ALPHA_BETA:
			return new AI_alpha_beta();
			break;
		case NEGA_SCOUT:
			return new AI_nega_scout();
			break;
		case MONTE_CARLO:
			return new AI_monte_carlo();
			break;
		case MONTECARLO_TREE:
			return new AI_montecarlo_tree();
			break;
		default:
			// return new AI_ordered();
			break;
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
public:
	enum {
		BOARD_SIZE = 3,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		//		mass_[0][0].setStatus(Mass::ENEMY); mass_[0][1].setStatus(Mass::PLAYER); 
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
					std::cout << "× ";
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

int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = b.calc_result();
	if (r == current) return +10000; // 呼び出し側の勝ち
	if (r == next) return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; //引き分け

	int score_max = -10001;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = b.mass_[x][y];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (score_max < score) {
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}

bool AI_nega_max::think(Board &b) {
	int best_x = -1, best_y;

	evaluate(b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) {
		return false;
	}

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -9999;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (beta < score) {
				return (score_max < score) ? score : score_max;
			}
			if (score_max < score) {
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha;
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}

bool AI_alpha_beta::think(Board& b) {
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999) {
		return false;
	}

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_nega_scout::evalueate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y) {
	if (limit-- == 0) {
		return 0;
	}

	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = board.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int a = alpha, b = beta;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evalueate(limit, -b, -a, board, next, dummy, dummy);
			if (a < score && score < beta && !(x == 0 && y == 0) && limit <= 2) {
				a = -evalueate(limit, -beta, -score, board, next, dummy, dummy);
			}
			m.setStatus(Mass::BLANK);

			if (a < score) {
				a = score;
				best_x = x;
				best_y = y;
			}

			if (beta <= a) {
				return a;
			}

			b = a + 1;
		}
	}

	return a;
}

bool AI_nega_scout::think(Board& b) {
	int best_x, best_y;

	if (evalueate(5, -10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999) {
		return false;
	}

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_monte_carlo::evaluate(bool first_time, Board& b, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int loses[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int blank_mass_num = 0;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = b.mass_[y][x];
			if (m.getStatus() == Mass::BLANK) {
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = loses[blank_mass_num] = 0;
				blank_mass_num++;
			}
		}
	}

	if (first_time) {
		for (int i = 0; i < 10000; i++) {
			int idx = rand() % blank_mass_num;
			Mass& m = b.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current);
			int dummy;
			int score = -evaluate(false, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (0 <= score) {
				wins[idx]++;
			} else {
				loses[idx]++;
			}
		}
		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++) {
			int score = wins[idx] + loses[idx];
			if (0 != score) {
				score = 100 * wins[idx] / score;
			}
			if (score_max < score) {
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " " << score << "% (win:" << wins[idx] << ", loses:" << loses[idx] << ")" << std::endl;
		}

		return score_max;
	}

	int idx = rand() % blank_mass_num;
	Mass& m = b.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);
	int dummy;
	int score = -evaluate(false, b, next, dummy, dummy);
	m.setStatus(Mass::BLANK);

	return score;
}

bool AI_monte_carlo::think(Board &b) {
	int best_x = -1, best_y;

	evaluate(true, b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false;

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_montecarlo_tree::evaluate(bool all_search, int count, Board& b, Mass::status current, int &best_x, int &best_y) {
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = b.calc_result();
	if (r == current) return +100;
	if (r == next) return -100;
	if (r == Board::DRAW) return 0;

	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int loses[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int scores[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int blank_mass_num = 0;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = b.mass_[y][x];
			if (m.getStatus() == Mass::BLANK) {
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = loses[blank_mass_num] = 0;
				blank_mass_num++;
			}
		}
	}

	if (all_search) {
		for (int i = 0; i < count; i++) {
			int idx = select_mass(blank_mass_num, loses, wins);
			if (idx < 0) break;
			Mass& m = b.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current);
			int dummy;
			int score = -evaluate(false, 0, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (0 < score) {
				wins[idx]++;
				loses[idx]++;
			}
			else {
				loses[idx]++;
			}

			if (count / 10 < loses[idx] && 10 < count) {
				m.setStatus(current);
				scores[idx] = 100 - evaluate(true, (int)sqrt(count), b, next, dummy, dummy);
				m.setStatus(Mass::BLANK);
				wins[idx] = -1;
			}
		}
		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++) {
			int score;
			if (-1 == wins[idx]) {
				score = scores[idx];
			}
			else if (0 == loses[idx]) {
				score = 0;
			}
			else {
				double c = 1. * sqrt(2 * log(count) / loses[idx]);
				score = 100 * wins[idx] / loses[idx] + (int)(c);
			}
			if (score_max < score) {
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			// std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " " << score << "% (win:" << wins[idx] << ", lose:" << loses[idx] << ")" << std::endl;
		}

		return score_max;
	}

	int idx = rand() % blank_mass_num;
	Mass& m = b.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);
	int dummy;
	int score = -evaluate(false, 0, b, next, dummy, dummy);
	m.setStatus(Mass::BLANK);

	return score;
}

int AI_montecarlo_tree::select_mass(int n, int* a_count, int* a_wins) {
	int total = 0;
	for (int i = 0; i < n; i++) {
		total += 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
	}
	if (total <= 0) return -1;

	int r = rand() % total;
	for (int i = 0; i < n; i++) {
		r -= 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
		if (r < 0) {
			return i;
		}
	}

	return -1;
}

bool AI_montecarlo_tree::think(Board& b) {
	int best_x = -1, best_y;

	if (will_win(b, best_x, best_y)) return b.mass_[best_y][best_x].put(Mass::ENEMY);
	if (will_lose(b, best_x, best_y)) return b.mass_[best_y][best_x].put(Mass::ENEMY);

	evaluate(true, 10000, b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false;

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

bool AI_montecarlo_tree::will_win(Board &b, int& best_x, int& best_y) {
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			if (b.mass_[y][x].getStatus() != Mass::BLANK) continue;

			b.mass_[y][x].setStatus(Mass::ENEMY);
			int r = b.calc_result();
			b.mass_[y][x].setStatus(Mass::BLANK);
			if (r == Mass::ENEMY) {
				best_x = x;
				best_y = y;
				return true;
			}
		}
	}

	return false;
}

bool AI_montecarlo_tree::will_lose(Board& b, int& best_x, int& best_y) {
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			if (b.mass_[y][x].getStatus() != Mass::BLANK) continue;

			b.mass_[y][x].setStatus(Mass::PLAYER);
			int r = b.calc_result();
			b.mass_[y][x].setStatus(Mass::BLANK);
			if (r == Mass::PLAYER) {
				best_x = x;
				best_y = y;
				return true;
			}
		}
	}

	return false;
}



class Game
{
private:
	const AI::type ai_type = AI::MONTECARLO_TREE;

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