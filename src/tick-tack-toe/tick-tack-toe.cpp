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
		TYPE_NEGA_MAX,
		TYPE_ALPHA_BETA,
		TYPE_NEGA_SCOUT,
		TYPE_MONTE_CARLO,
		TYPE_MONTECARLO_TREE,
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
	int evaluate(Board& b, Mass::status next, int& best_x, int& best_y);
public:
	AI_nega_max() {}
	~AI_nega_max() {}

	bool think(Board& b);
};

class AI_alpha_beta :public AI {
private:
	int evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_alpha_beta() {}
	~AI_alpha_beta() {}

	bool think(Board& b);
};

class AI_nega_scout :public AI {
private:
	int evaluate(int limit, int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_nega_scout() {}
	~AI_nega_scout() {}

	bool think(Board& b);
};

class AI_monte_carlo :public AI {
private:
	int evaluate(bool first_time, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_monte_carlo() {}
	~AI_monte_carlo() {}

	bool think(Board& b);
};

class AI_montecarlo_tree :public AI {
private:
	static int select_mass(int n, int* a_cout, int* a_wins);
	bool check_win(Board& b,int &best_x,int &best_y);
	bool check_lose(Board& b,int &best_x,int &best_y);
	int evaluate(bool all_seach, int count, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_montecarlo_tree() {}
	~AI_montecarlo_tree() {}

	bool think(Board& b);

};



AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_NEGA_MAX:
		return new AI_nega_max();
		break;
	case TYPE_ALPHA_BETA:
		return new AI_alpha_beta();
		break;
	case TYPE_NEGA_SCOUT:
		return new AI_nega_scout();
		break;
	case TYPE_MONTE_CARLO:
		return new AI_monte_carlo();
		break;
	case TYPE_MONTECARLO_TREE:
		return new AI_montecarlo_tree();
		break;
	default:
		return new AI_ordered();
		break;
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered;
	friend class AI_nega_max;
	friend class AI_alpha_beta;
	friend class AI_nega_scout;
	friend class AI_monte_carlo;
	friend class AI_montecarlo_tree;

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
	static const int BOARD_SIZE_HALF = BOARD_SIZE / 2;
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		// mass_[0][0].setStatus(Mass::ENEMY); mass_[0][1].setStatus(Mass::PLAYER);
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

int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = b.calc_result();
	if (r == current)return +10000; // 呼び出し側の勝ち
	if (r == next)return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	int score_max = -10001; // 打たないのは最悪

	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK); // 手を戻す

			if (score_max < score) {
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}

bool AI_nega_max::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0)return false; // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);

}

int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = b.calc_result();
	if (r == current)return +10000; // 呼び出し側の勝ち
	if (r == next)return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	int score_max = -9999; // 打たないで投了

	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK); // 手を戻す

			if (beta < score) {
				return (score_max < score) ? score : score_max; // 最悪の値より悪い
			}


			if (score_max < score) {
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha; // α値を更新
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}

bool AI_alpha_beta::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false; // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_nega_scout::evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y)
{
	if (limit-- == 0)return 0; // 深さ制限に達した

	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = board.calc_result();
	if (r == current)return +10000; // 呼び出し側の勝ち
	if (r == next)return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	int a = alpha, b = beta;
	bool is_first = true;

	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(limit, -b, -a, board, next, dummy, dummy);
			if (a < score && score < beta && !is_first || 2 <= limit)
			{
				score = -evaluate(limit, -beta, -score, board, next, dummy, dummy);
			}
			is_first = false;

			m.setStatus(Mass::BLANK); // 手を戻す

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

bool AI_nega_scout::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(5, -10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
	{
		return false; // 打てる手はなかった
	}

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_monte_carlo::evaluate(bool first_time, Board& board, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	// 死活判定
	int r = board.calc_result();
	if (r == current)return +10000; // 呼び出し側の勝ち
	if (r == next)return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int loses[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int blank_mass_num = 0;

	// 空いているマスの数を数え、配列として位置を確保する
	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() == Mass::BLANK)
			{
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = loses[blank_mass_num] = 0;
				blank_mass_num++;
			}
		}
	}
	if (first_time) {
		// 一番上の階層でランダムに指すのを繰り返す
		for (int i = 0;i < 10000;i++) {
			int idx = rand() % blank_mass_num;
			Mass& m = board.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(false, board, next, dummy, dummy);
			m.setStatus(Mass::BLANK);// 手を戻す

			if (0 <= score) {
				wins[idx]++;
			}
			else {
				loses[idx]++;
			}
		}
		int score_max = -9999;
		for (int idx = 0;idx < blank_mass_num;idx++) {
			int score = wins[idx] + loses[idx];
			if (0 != score) {
				score = 100 * wins[idx] / score;// 勝率
			}
			if (score_max < score) {
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " " << score << "% (win:" << wins[idx];
			std::cout << ",lose:" << loses[idx] << ")" << std::endl;
		}
		return score_max;
	}

	int idx = rand() % blank_mass_num;
	Mass& m = board.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);// 次の手を打つ
	int dummy;
	int score = -evaluate(false, board, next, dummy, dummy);
	m.setStatus(Mass::BLANK);// 手を戻す

	return score;
}

bool AI_monte_carlo::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(true, b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false; // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_montecarlo_tree::evaluate(bool all_seach, int sim_count, Board& board, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	// 死活判定
	int r = board.calc_result();
	if (r == current)return +10000; // 呼び出し側の勝ち
	if (r == next)return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];// 勝利数
	int count[Board::BOARD_SIZE * Board::BOARD_SIZE];// 敗北数
	int scores[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int blank_mass_num = 0;

	// 空いているマスの数を数え、配列として位置を確保する
	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() == Mass::BLANK)
			{
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = count[blank_mass_num] = 0;
				scores[blank_mass_num] = -1;
				blank_mass_num++;
			}
		}
	}
	if (all_seach) {
		// 一番上の階層でランダムに指すのを繰り返す
		for (int i = 0;i < 10000;i++) {
			int idx = select_mass(blank_mass_num, count, wins);
			if (idx < 0)break;
			Mass& m = board.mass_[y_table[idx]][x_table[idx]];

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(false, 0, board, next, dummy, dummy);
			m.setStatus(Mass::BLANK);// 手を戻す

			if (0 <= score) {
				wins[idx]++;
				count[idx]++;
			}
			else {
				count[idx]++;
			}
			// 閾値を超えれば、木を成長させる
			if (sim_count / 10 < count[idx]// 閾値は10％以上の探索回数
				&& 10 < sim_count) {// 回数が少ないとはランダムの精度が下がるので、成長させない
				m.setStatus(current);// 次の手を打つ
				scores[idx] = 100 - evaluate(true, (int)sqrt(sim_count), board, next, dummy, dummy);
				m.setStatus(Mass::BLANK);// 手を戻す
				wins[idx] = -1;// この枝は乱数で呼ばれないようにする
			}

		}
		int score_max = -9999;
		for (int idx = 0;idx < blank_mass_num;idx++) {
			int score;
			if (-1 == wins[idx]) {
				score = scores[idx];// 枝分かれした
			}
			else if (0 == count[idx]) {
				score = 0;// 一度も通らなかった
			}
			else {
				double c = 1.0 * sqrt(2 * log(sim_count) / count[idx]);
				score = 100 * wins[idx] / count[idx] + (int)(c);// 勝率
			}
			if (score_max < score) {
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " ";
			std::cout << score << "% (win:" << wins[idx] << ",lose:" << count[idx] << ")" << std::endl;
		}
		return score_max;
	}

	int idx = rand() % blank_mass_num;
	Mass& m = board.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);// 次の手を打つ
	int dummy;
	int score = -evaluate(false, 0, board, next, dummy, dummy);
	m.setStatus(Mass::BLANK);// 手を戻す

	return score;
}

// 勝率が高いほど多く割り当てる
int AI_montecarlo_tree::select_mass(int n, int* a_count, int* a_wins)
{
	int total = 0;
	for (int i = 0;i < n;i++) {
		total += 10000 * (a_wins[i] + 1) / (a_count[i] + 1);// 0の時にも確率があるように+1する
	}
	if (total <= 0)return -1;

	int r = rand() % total;
	for (int i = 0;i < n;i++) {
		r -= 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
		if (r < 0) {
			return i;
		}
	}
	return -1;
}

bool AI_montecarlo_tree::check_win(Board& board,int &best_x,int &best_y)
{
	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(Mass::ENEMY); // 次の手を打つ
			if (board.calc_result() == Mass::ENEMY)
			{
				best_x = x;
				best_y = y;
				std::cout << "勝てる場所に置く" << std::endl;
				m.setStatus(Mass::BLANK); // 手を戻す
				return true;
			}
			m.setStatus(Mass::BLANK); // 手を戻す
		}
	}
	return false;
}

bool AI_montecarlo_tree::check_lose(Board& board, int& best_x, int& best_y)
{
	for (int y = 0;y < Board::BOARD_SIZE; y++) {
		for (int x = 0;x < Board::BOARD_SIZE;x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(Mass::PLAYER); // 次の手を打つ
			if (board.calc_result() == Mass::PLAYER)
			{
				best_x = x;
				best_y = y;
				std::cout << "負けないように置く" << std::endl;
				m.setStatus(Mass::BLANK); // 手を戻す

				return true;
			}
			m.setStatus(Mass::BLANK); // 手を戻す

		}
	}
	return false;
}

bool AI_montecarlo_tree::think(Board& b)
{
	int best_x = -1, best_y;

	if (b.mass_[Board::BOARD_SIZE_HALF][Board::BOARD_SIZE_HALF].getStatus() == Mass::BLANK)
		return b.mass_[Board::BOARD_SIZE_HALF][Board::BOARD_SIZE_HALF].put(Mass::ENEMY);

	if (check_win(b, best_x, best_y))
		return b.mass_[best_y][best_x].put(Mass::ENEMY);

	if (check_lose(b, best_x, best_y))
		return b.mass_[best_y][best_x].put(Mass::ENEMY);

	evaluate(true, 10000, b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false; // 打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

class Game
{
private:
	//const AI::type ai_type = AI::TYPE_ORDERED;
	//const AI::type ai_type = AI::TYPE_NEGA_MAX;
	//const AI::type ai_type = AI::TYPE_ALPHA_BETA;
	//const AI::type ai_type = AI::TYPE_NEGA_SCOUT;
	//const AI::type ai_type = AI::TYPE_MONTE_CARLO;
	const AI::type ai_type = AI::TYPE_MONTECARLO_TREE;

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
	std::cout << "if input 0 0 next game  " << std::endl;
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
					if (col[0] == '0' && row[0] == '0')
					{
						winner = Board::DRAW;
						break;
					}
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				// AI
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);// 投了
					break;
				}
				std::cout << std::endl;
			}
			if (winner == Board::DRAW)
				break;
			// プレイヤーとAIの切り替え
			turn = 1 - turn;
		}
	}
	return 0;
}