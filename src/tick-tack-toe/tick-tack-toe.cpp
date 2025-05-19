#include <memory>
#include <iostream>
#include <vector>
#include <list>
#include <utility>


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
		TYPE_MONTECARLO,
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

//Nega-Max法のAI
class AI_nega_max :public AI
{
private:
	int evaluate(Board& b, Mass::status current,int &best_x,int &best_y);
public:
	AI_nega_max() {}//コンストラクタ
	~AI_nega_max() {}//デストラクタ

	bool think(Board& b);//AIの思考
};

//Alpha-Beta法のAI
class AI_alpha_beta :public AI
{
private:
	int evaluate(int alpha,int beta,Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_alpha_beta() {}//コンストラクタ
	~AI_alpha_beta() {}//デストラクタ

	bool think(Board& b);//AIの思考
};

//モンテカルロ法のAI
class AI_montecarlo :public AI
{
private:
	//playOutloadControlがfalseの時...どれだけ空きマスがあったとしてもsimulationCount回プレイアウトを行う
	//playOutloadControlがtrueの時...空きマスの数に応じてプレイアウト回数を調整(simulationCount/残り空きマス数回プレイアウトを行う)
	const int simulationCount = 500000;//プレイアウトをどれだけするか
	const bool playOutLoadControl=true;//これがtrueになっていると残り空きマスの数に応じてプレイアウトの回数を調整するようになる(常にプレイアウトの処理負荷が一定になる)
	const float winCoefficient = 1.0f;//評価時の勝利率にかける係数
	const float drawCoefficient = 1.0f;//評価時の引き分け率にかける係数
	int random_PlayOut(std::list<std::pair<int, int>> blinkMassList, Board b, int& index_firstMass);//プレイアウト

	struct PlayOutMass
	{
		int x = -1;
		int y = -1;
		int winCount = 0;//勝利数
		int loseCount = 0;//敗北数
		int playCount = 0;//プレイ数
		//(プレイ数ー勝利数ー敗北数で引き分け数が求まるはず)
	};

public:
	AI_montecarlo() {}//コンストラクタ
	~AI_montecarlo() {}//デストラクタ

	bool think(Board& b);//AIの思考
};

AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_NEGA_MAX://Nega-Max法のAI
		return new AI_nega_max();
	case TYPE_ALPHA_BETA://Alpha-Beta法のAI
		return new AI_alpha_beta();
	case TYPE_MONTECARLO://モンテカルロ法のAI
		return new AI_montecarlo();
	default://順番にうっていくやつ
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
	friend class AI_montecarlo;
	friend class Game;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};

	enum {
		BOARD_SIZE = 3,
	};
private:
	
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




//☆順番にうっていくやつ

//順番に空いてるところに置く
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



//☆Nega-Max関係

int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	//死活判定
	int result = b.calc_result();
	if (result == current) return +10000;//呼び出し側の勝ち
	if (result == next) return -10000;//呼び出し側の負け
	if (result == Board::DRAW) return 0;//引き分け

	int score_max = -10001;//打たないのは最悪

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;//そのマスが空白でなければ処理はそのマスの処理はやらない

			//評価値を算出
			m.setStatus(current);//仮に次の手を打つ
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);//元に戻しておく

			//最大値の確保＆手の記録
			if (score > score_max)
			{
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}

//局面ごとに思考する
bool AI_nega_max::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false;

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}




//alpha-beta法関係

int AI_alpha_beta::evaluate(int alpha,int beta,Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	//死活判定
	int result = b.calc_result();
	if (result == current) return +10000;//呼び出し側の勝ち
	if (result == next) return -10000;//呼び出し側の負け
	if (result == Board::DRAW) return 0;//引き分け

	int score_max = -9999;//打たないで終了

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;//そのマスが空白でなければ処理はそのマスの処理はやらない

			//評価値を算出
			m.setStatus(current);//仮に次の手を打つ
			int dummy;
			int score = -evaluate(-beta,-alpha,b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);//元に戻しておく

			if (beta < score)
			{
				return (score_max < score) ? score : score_max;//最悪の値より悪い
			}

			//最大値の確保＆手の記録
			if (score > score_max)
			{
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha;//α値を更新
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}

//局面ごとに思考する
bool AI_alpha_beta::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false;//打てる手はなかった

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}




//☆モンテカルロ法

//プレイアウト
int AI_montecarlo::random_PlayOut(std::list<std::pair<int, int>> blinkMassList, Board b, int& index_firstMass)
{
	//最初はAIのターン
	Mass::status turn = Mass::ENEMY;

	bool firstTurn = true;//最初のターンか？

	while (b.calc_result() == Board::NOT_FINISED)//ゲームが終わるか引き分けになるまで続く
	{
		if (blinkMassList.empty()) break;//空きマスリストに空きがなければここで処理を終了

		//置くマスを決める
		int idx;
		idx = rand() % blinkMassList.size();
		auto it = std::next(blinkMassList.begin(), idx);
		int x = it->first;
		int y = it->second;

		if (firstTurn)//最初のターンなら
		{
			index_firstMass = idx;//最初に置くマスを伝える
			firstTurn = false;
		}

		//置く
		b.mass_[y][x].setStatus(turn);

		//空きマスリストから置いたマスを削除
		blinkMassList.erase(it);

		//ターンを交代
		turn = (turn == Mass::PLAYER) ? Mass::ENEMY : Mass::PLAYER;
	}

	//決着がついたら勝者を返す
	return (int)b.calc_result();

}


//思考
bool AI_montecarlo::think(Board& b)
{
	//空きマスリストを作る
	std::list<std::pair<int, int>> blinkMassList;//空きマスリスト(左がx、右がy)

	for (int y = 0; y < b.BOARD_SIZE; y++)
	{
		for (int x = 0; x < b.BOARD_SIZE; x++)
		{
			if (b.mass_[y][x].getStatus() == Mass::BLANK)
			{
				blinkMassList.emplace_back(x, y);
			}
		}
	}

	if (blinkMassList.empty()) return false;//空きマスが無かった場合

	//調査したマスを記録するリストを作る(空きマスリストと要素番号と格納されているマスの位置を一致させる)
	std::vector<PlayOutMass> candidates;//調査したマスの記録リスト

	for (auto it = blinkMassList.begin(); it!= blinkMassList.end() ;it++)
	{
		int x = it->first;
		int y = it->second;
		PlayOutMass candidateMass{ x,y };

		candidates.push_back(candidateMass);
	}

	//空いてるマスをプレイアウトで調査する
	//最初に置いたマスの要素番号(空きマスリスト上の)とその結果勝ったかを教えてもらう

	//プレイアウト回数の決定
	//playOutLoadControlがtrueになっているなら残り空きマスの数に応じてプレイアウト回数を調整
	//falseならsimlationCount回数分そのままプレイアウトする
	int playOutCount = (playOutLoadControl) ? simulationCount / blinkMassList.size() : simulationCount;

	for (int i = 0; i < playOutCount; i++)
	{
		int idx;

		int winner = random_PlayOut(blinkMassList, b, idx);//プレイアウトする

		if ((Board::WINNER)winner==Board::ENEMY) candidates[idx].winCount++;//勝利していたら勝利カウントを増やす
		if ((Board::WINNER)winner == Board::PLAYER) candidates[idx].loseCount++;//敗北していたら敗北カウントを増やす
		candidates[idx].playCount++;//プレイ回数を増やす
	}

	//最も勝率の高い手を選ぶ
	PlayOutMass bestMass;
	float bestScore = -1;

	for (int i = 0; i < candidates.size(); i++)
	{
		if (candidates[i].playCount == 0) continue;//一度も置いていないマスは飛ばす

		//勝率を求める
		float winRate = (float)candidates[i].winCount / candidates[i].playCount;

		//引き分け率を求める
		int drawCount = candidates[i].playCount - candidates[i].loseCount - candidates[i].winCount;
		float drawRate = (float)drawCount / candidates[i].playCount;

		//勝率と引き分け率をもとにスコアを算出
		float score = winRate * winCoefficient + drawRate * drawCoefficient;

		if (score > bestScore)//最大値を更新
		{
			bestScore = score;
			bestMass = candidates[i];
		}
	}

	if (bestScore == -1) return false;//一度もプレイアウトしていなくて最大勝率が初期値(-1)のままの場合

	return b.mass_[bestMass.y][bestMass.x].put(Mass::ENEMY);
}


class Game
{
private:
	const AI::type ai_type = AI::TYPE_MONTECARLO;
	const int borderBlinkMassCount_ChangeToAlphaBeta = 5;//モンテカルロ法からalpha-beta法に変更する判断をする空きマスの数(空きマスの数がこの数以下になるとalpha-beta法になる)

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
		//残り空きマスが少なくなったらモンテカルロ法からalpha-beta法に変更

		//空きマスを数える
		int blinkMassCount = 0;

		for (int y = 0; y < board_.BOARD_SIZE; y++)
		{
			for (int x = 0; x < board_.BOARD_SIZE; x++)
			{
				if (board_.mass_[y][x].getStatus() == Mass::BLANK)
				{
					blinkMassCount++;
				}
			}
		}

		//空きマス数が一定以下になるとモンテカルロ法からalpha-beta法に変更
		if (blinkMassCount <= borderBlinkMassCount_ChangeToAlphaBeta)
		{
			pAI_ = AI::createAi(AI::TYPE_ALPHA_BETA);
			std::cout << "AI:ここから本気出すね～" << std::endl;
		}


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