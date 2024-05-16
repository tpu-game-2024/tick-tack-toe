#include <memory>
#include <iostream>
#include <unordered_map> 
#include <string> 


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
		TYPE_ALPHA_BETA = 1,
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


class Board
{
	friend class AI_ordered;
	friend class AI_alpha_beta;

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

class AI_alpha_beta : public AI
{
private:
	std::unordered_map<std::string, int> evaluationCache;

	int evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_alpha_beta() {}
	~AI_alpha_beta() {}

	bool find_cache(Board& board, int& score);
	void rotate90(Mass board[][Board::BOARD_SIZE]);
	void flip(Mass board[][Board::BOARD_SIZE]);
	std::string to_string(Mass board[][Board::BOARD_SIZE]);

	bool think(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type)
	{
		// case TYPE_ORDERED:
		case type::TYPE_ALPHA_BETA:
			return new AI_alpha_beta();
			break;
		default:
			return new AI_ordered();
			break;
	}

	return nullptr;
}


void AI_alpha_beta::rotate90(Mass board[][Board::BOARD_SIZE])
{
	Mass newBoard[Board::BOARD_SIZE][Board::BOARD_SIZE];

	memcpy((void*)newBoard, (void*)board, sizeof(Mass) * Board::BOARD_SIZE * Board::BOARD_SIZE);

	for (int y = 0; y < Board::BOARD_SIZE; ++y)
	{
		for (int x = 0; x < Board::BOARD_SIZE; ++x)
		{
			board[y][x] = newBoard[x][Board::BOARD_SIZE - y - 1];
		}
	}
}

void AI_alpha_beta::flip(Mass board[][Board::BOARD_SIZE])
{
	for (int y = 0; y < Board::BOARD_SIZE; ++y)
	{
		for (int x = 0; x < Board::BOARD_SIZE / 2; ++x)
		{
			board[y][x] = board[y][Board::BOARD_SIZE - x - 1];
		}
	}
}


std::string AI_alpha_beta::to_string(Mass board[][Board::BOARD_SIZE])
{
	std::string str;

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			str += '0' + board[y][x].getStatus();
		}
	}

	return str;
}

bool AI_alpha_beta::find_cache(Board& board, int& score)
{
	Mass current[Board::BOARD_SIZE][Board::BOARD_SIZE];

	memcpy((void*)current, (void*)board.mass_, sizeof(Mass) * Board::BOARD_SIZE * Board::BOARD_SIZE);

	bool is_flip = false;

	// 線対称 + 点対称で8パターン
	for (int i = 0; i < 8; ++i)
	{
		if (is_flip)
		{
			// 線対称の盤面を取得
			flip(current);
		}
		else
		{
			// 点対称の盤面を取得
			rotate90(current);
		}

		is_flip = !is_flip;

		std::string key = to_string(current);

		// キャッシュされてたらそのスコアを取得 	
		if (evaluationCache.find(key) != evaluationCache.end())
		{
			score = evaluationCache[key];
			return true;
		}
	}

	return false;
}

bool AI_alpha_beta::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
	{
		return false;
	}

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -9999; // 打たないで投了
	std::string current_key; // 現在の盤面のキャッシュキー

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);

			int score;
			int dummy;
			current_key = to_string(b.mass_);

			//キャッシュされてる評価値を取得 
			if (!find_cache(b, score))
			{
				score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			}

			m.setStatus(Mass::BLANK);

			if (beta < score)
			{
				int result = (score_max < score) ? score : score_max;
				evaluationCache[current_key] = result;
				return result;
			}
			if (score_max < score)
			{
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha;
				best_x = x;
				best_y = y;
				evaluationCache[current_key] = score_max;
			}
		}
	}

	return score_max;
}

class Game
{
private:
	const AI::type ai_type = AI::TYPE_ALPHA_BETA;

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