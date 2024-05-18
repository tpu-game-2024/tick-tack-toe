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
		TYPE_SUPER
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
	int evaluate(Board& b, Mass::status next, int& x, int& y);
public:
	AI_nega_max() {}
	~AI_nega_max() {}

	bool think(Board& b);
};
class AI_alpha_beta : public AI {
private:
	int evaluate(int alpha, int beta, Board& b, Mass::status next, int& x, int& y);
public:
	AI_alpha_beta() {}
	~AI_alpha_beta() {}

	bool think(Board& b);
};
class AI_nega_scout : public AI {
private:
	int evaluate(int limit, int alpha, int beta, Board& b, Mass::status next, int& x, int& y);
public:
	AI_nega_scout() {}
	~AI_nega_scout() {}

	bool think(Board& b);
};
class AI_super : public AI {
private:
	int evaluate(int limit, int alpha, int beta, Board& b, Mass::status next, int& x, int& y);
public:
	AI_super() {}
	~AI_super() {}

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
	case TYPE_SUPER:
		return new AI_super();
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
	friend class AI_super;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
	int steps = 0;
private:
	enum {
		BOARD_SIZE = 5,
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
			if (x == BOARD_SIZE) { int steps = 0; return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { int steps = 0; return(Board::WINNER)winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { int steps = 0; return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { int steps = 0; return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISED;
			}
		}
		int steps = 0; return DRAW;
	}

	bool put(int x, int y) {
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		steps++;
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
bool AI_nega_max::think(Board& b)
{
	int best_x = -1, best_y;
	evaluate(b, Mass::ENEMY, best_x, best_y);
	if (best_x < 0)return false;
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}
bool AI_alpha_beta::think(Board& b)
{
	int best_x = -1, best_y;
	evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y);
	if (best_x < 0)return false;
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}
bool AI_nega_scout::think(Board& b)
{
	int best_x = -1, best_y;
	evaluate(5, -10000, 10000, b, Mass::ENEMY, best_x, best_y);
	if (best_x < 0)return false;
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}
bool AI_super::think(Board& b)
{
	int best_x = -1, best_y;
	evaluate(5, -10000, 10000, b, Mass::ENEMY, best_x, best_y);
	if (best_x < 0)
		return false;
	b.steps++;
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}
int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -10001;
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{

			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (score_max < score)
			{
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}
int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	int trial = 0;
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -10001;
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{

			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);
			trial++;
			std::cout << trial << std::endl;
			if (beta < score)
			{
				return (score_max < score) ? score : score_max;
			}
			if (score_max < score)
			{
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}

int AI_nega_scout::evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y)
{

	if (limit == 0) return 0;
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	int r = board.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;
	int a = alpha;
	int b = beta;
	int score_max = -10001;
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{

			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(limit, -b, -a, board, next, dummy, dummy);
			if (a < score && score < b && !(x == 0 && y == 0) && limit <= 2)
			{
				a = -evaluate(limit, -beta, -alpha, board, next, dummy, dummy);
			}
			m.setStatus(Mass::BLANK);
			if (a < score)
			{
				a = score;
				best_x = x;
				best_y = y;
			}
			if (beta <= a)
			{
				return a;
			}
			b = a + 1;
		}
	}
	return a;
}
int AI_super::evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y)
{
	int WinBoard[Board::BOARD_SIZE][Board::BOARD_SIZE];
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			WinBoard[y][x] = 0;
		}
	}
	if (board.steps <= (Board::BOARD_SIZE * Board::BOARD_SIZE) / 2)
	{
		const int voidScore = 1;
		const int PlayerScore = 2;
		const int EnemyScore = 3;
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			int GetScore = 0;
			int isPlayer = 0;
			int isEnemy = 0;
			int isVoid = 0;
			int x = 0;
			for (; x < Board::BOARD_SIZE; x++)
			{
				switch (board.mass_[y][x].getStatus())
				{
				case Mass::PLAYER:
					isPlayer++;
					GetScore += PlayerScore;
					WinBoard[y][x] = -1;
					break;
				case Mass::ENEMY:
					isEnemy++;
					GetScore += EnemyScore;
					WinBoard[y][x] = -1;
					break;
				default:
					isVoid++;
					GetScore += voidScore;
					break;
				}
				if (isPlayer > 0 && isEnemy > 0)
				{

					break;
				}

			}
			if (x == Board::BOARD_SIZE)
			{


				if (isEnemy >= Board::BOARD_SIZE - 1)
					GetScore = 1000;

				else if (isPlayer >= Board::BOARD_SIZE - 1)
					GetScore = 100;

				for (int x = 0; x < Board::BOARD_SIZE; x++)
				{
					if (WinBoard[y][x] >= 0)
						WinBoard[y][x] += GetScore;
				}
			}
		}
		// 縦

		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			int GetScore = 0;
			int isPlayer = 0;
			int isEnemy = 0;
			int isVoid = 0;
			int y = 0;
			for (; y < Board::BOARD_SIZE; y++)
			{
				switch (board.mass_[y][x].getStatus())
				{
				case Mass::PLAYER:
					isPlayer++;
					GetScore += PlayerScore;
					WinBoard[y][x] = -1;
					break;
				case Mass::ENEMY:
					isEnemy++;
					GetScore += EnemyScore;
					WinBoard[y][x] = -1;
					break;
				default:
					isVoid++;
					GetScore += voidScore;
					break;
				}
				if (isPlayer > 0 && isEnemy > 0)
				{
					break;
				}
			}
			if (y == Board::BOARD_SIZE)
			{

				if (isEnemy >= Board::BOARD_SIZE - 1)
					GetScore = 1000;

				else if (isPlayer >= Board::BOARD_SIZE - 1)
					GetScore = 100;

				for (int y = 0; y < Board::BOARD_SIZE; y++)
				{
					if (WinBoard[y][x] >= 0)
						WinBoard[y][x] += GetScore;
				}
			}
		}

		/*斜め*/ {
			int GetScore = 0;
			int isPlayer = 0;
			int isEnemy = 0;
			int isVoid = 0;
			int i = 0;
			for (; i < Board::BOARD_SIZE; i++)
			{
				switch (board.mass_[i][i].getStatus())
				{
				case Mass::PLAYER:
					isPlayer++;
					GetScore += PlayerScore;
					WinBoard[i][i] = -1;
					break;
				case Mass::ENEMY:
					isEnemy++;
					GetScore += EnemyScore;
					WinBoard[i][i] = -1;
					break;
				default:
					isVoid++;
					GetScore += voidScore;
					break;
				}
				if (isPlayer > 0 && isEnemy > 0)
				{
					break;
				}
			}
			if (i == Board::BOARD_SIZE)
			{


				if (isEnemy >= Board::BOARD_SIZE - 1)
					GetScore = 1000;

				else if (isPlayer >= Board::BOARD_SIZE - 1)
					GetScore = 100;

				for (int i = 0; i < Board::BOARD_SIZE; i++)
				{
					if (WinBoard[i][i] >= 0)
						WinBoard[i][i] += GetScore;
				}
			}

		}

		/*逆斜め*/ {
			int GetScore = 0;
			int isPlayer = 0;
			int isEnemy = 0;
			int isVoid = 0;
			int i = 0;
			for (; i < Board::BOARD_SIZE; i++)
			{
				switch (board.mass_[Board::BOARD_SIZE - 1 - i][i].getStatus())
				{
				case Mass::PLAYER:
					isPlayer++;
					GetScore += PlayerScore;
					WinBoard[i][i] = -1;
					break;
				case Mass::ENEMY:
					isEnemy++;
					GetScore += EnemyScore;
					WinBoard[i][i] = -1;
					break;
				default:
					isVoid += 1;
					GetScore += voidScore;
					break;
				}

			}
			if (i == Board::BOARD_SIZE)
			{


				if (isEnemy >= Board::BOARD_SIZE - 1)
					GetScore = 1000;

				else if (isPlayer >= Board::BOARD_SIZE - 1)
					GetScore = 100;

				for (int i = 0; i < Board::BOARD_SIZE; i++)
				{
					if (WinBoard[Board::BOARD_SIZE - 1 - i][i] >= 0)
						WinBoard[Board::BOARD_SIZE - 1 - i][i] += GetScore;
				}
			}
		}

		int bestScore = 0;
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			for (int x = 0; x < Board::BOARD_SIZE; x++)
			{
				std::cout << WinBoard[y][x];
				std::cout << " ";
				if (WinBoard[y][x] >= bestScore)
				{
					best_x = x;
					best_y = y;
					bestScore = WinBoard[y][x];
				}
			}
			std::cout << "\n";
		}
		return bestScore;
	}
	else
	{
		if (limit == 0) return 0;
		Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
		int r = board.calc_result();
		if (r == current) return +10000;
		if (r == next) return -10000;
		if (r == Board::DRAW) return 0;
		int a = alpha;
		int b = beta;
		int score_max = -10001;
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			for (int x = 0; x < Board::BOARD_SIZE; x++)
			{

				Mass& m = board.mass_[y][x];
				if (m.getStatus() != Mass::BLANK)continue;

				m.setStatus(current);
				int dummy;
				int score = -evaluate(limit, -b, -a, board, next, dummy, dummy);
				if (a < score && score < b && !(x == 0 && y == 0) && limit <= 2)
				{
					a = -evaluate(limit, -beta, -alpha, board, next, dummy, dummy);
				}
				m.setStatus(Mass::BLANK);
				if (a < score)
				{
					a = score;
					best_x = x;
					best_y = y;
				}
				if (beta <= a)
				{
					return a;
				}
				b = a + 1;
			}
		}
		return a;
	}

}
class Game
{
private:
	const AI::type ai_type = AI::TYPE_SUPER;

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