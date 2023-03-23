#pragma once

#include "resource.h"
#include "Resource.h"
#include <vector>
#include <cassert>

class MyBitMap {
	int _id = 0;
	HBITMAP _bitmap = NULL;
	BITMAP _info;
public:
	MyBitMap() = delete;
	MyBitMap(int id_) : _id(id_) { init(_id); };

	void init(int id) {
		_bitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(id));
		const auto& result = GetObject(_bitmap, sizeof(_info), &_info);
		if (!result) { auto e = GetLastError(); }
	}

	void loadFromResource(int id) {
		_bitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(id));
	}

	const BITMAP& info() const { return _info; }
	const size_t& width() const { return static_cast<size_t>(_info.bmWidth); }
	const size_t& height() const { return static_cast<size_t>(_info.bmHeight); }

	void draw(HWND hWnd_, HDC hdc_, int x_, int y_) const {
		if (!_bitmap) { return; }
		if (!hWnd_) { return; }
		HDC srcDC = GetDC(hWnd_);
		SelectObject(srcDC, _bitmap);
		BitBlt(hdc_, x_, y_, (int)width(), (int)height(), srcDC, 0, 0, SRCCOPY);
	}
};

class Util {
public:

	static int getRandomInt(const int from, const int to) {
		int dist = to - from + 1;
		return from + (rand() % dist);
	}
	
	static POINT getRandomPointInRect(RECT& r) {
		int randX = getRandomInt(r.left, r.right);
		int randY = getRandomInt(r.top, r.bottom);
		POINT p{ randX, randY };
		return p;
	}
};

class Painter {

public:
	static void drawSquare(HDC hdc_, POINT pos_, size_t size_, COLORREF color)
	{
		SelectObject(hdc_, GetStockObject(DC_BRUSH));
		SetDCBrushColor(hdc_, color);
		Rectangle(hdc_, pos_.x, pos_.y, pos_.x + static_cast<int>(size_), pos_.y + static_cast<int>(size_));
	}

	static void drawScene_Landing(HWND hWnd_, HDC hdc_) {
		RECT tr;
		GetClientRect(hWnd_, &tr);
		LONG tr_h = tr.bottom - tr.top;
		LONG tr_w = tr.right - tr.left;
		tr.top += tr_h / 4;
		tr.bottom -= tr_h / 2;
		tr.left += tr_w / 4;
		tr.right -= tr_w / 4;


		HPEN tPen = CreatePen(PS_SOLID, 5, RGB(127, 127, 127));
		SelectObject(hdc_, GetStockObject(WHITE_BRUSH));
		SelectObject(hdc_, tPen);
		Rectangle(hdc_, tr.left, tr.top, tr.right, tr.bottom);
		DrawText(hdc_, TEXT("SNAKE"), 5, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DeleteObject(tPen);
	}

	static void drawMessage(HWND hWnd_, HDC hdc_, const wchar_t* s, RECT rect_, COLORREF bgColor = TRANSPARENT, COLORREF textColor = RGB(0, 0, 0)) { //transparent become black..?
		SetTextColor(hdc_, textColor);
		SetBkColor(hdc_, bgColor);
		DrawText(hdc_, s, (int)wcslen(s), &rect_, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}


};


class GameObject {

protected:
	POINT _pos{ 0, 0 };
	size_t _size = 30;

public:
	//GameObject() = delete;
	GameObject(int x = 0, int y = 0, size_t size_ = 30)
	{
		_pos.x = x;
		_pos.y = y;
		_size = size_;
	}
	~GameObject() {
		reset();
	}
	void resetPos() { setPos(0, 0); }
	void resetSize() { setSize(0); }
	
	virtual void reset() { 
		resetPos();
		resetSize();
	};
	
	const POINT& getPos() const { return _pos; }
	POINT getPos() { return _pos; }
	
	POINT getNextPos(const int x, const int y) const {
		POINT p = getPos();
		p.x += x;
		p.y += y;
		return  p;
	}
	
	void setPos(const int x_, const int y_) { _pos.x = x_; _pos.y = y_; }
	void setPos(const POINT pos_) { setPos(pos_.x, pos_.y); }
	
	
	size_t getSize() const 			{ return _size; };
	void setSize(const size_t size) { _size = size; };

	RECT hitBox() const {
		RECT r{ _pos.x, _pos.y, _pos.x + static_cast<LONG>(_size), _pos.y + static_cast<LONG>(_size)};
		return r;
	}

	bool isCollided(const GameObject& other) const { 
		RECT t;
		RECT thisBox = hitBox();
		RECT otherBox = other.hitBox();
		return IntersectRect(&t, &thisBox, &otherBox) != 0;
	}

};



class SnakeBody : GameObject {
	friend class Snake;
	friend class Game;

public:	
	SnakeBody(int x_, int y_, size_t size_ = 30) : GameObject(x_, y_, size_) { }
	SnakeBody(const POINT& pos_, size_t size_ = 30) : SnakeBody(pos_.x, pos_.y, size_) { }
	
	~SnakeBody(){	
		printf("SnakeBody dtor: pos[x: %d, y: %d]", _pos.x, _pos.y);
		reset();
	}
	
	
	void draw(HDC hdc_, COLORREF color_) const { 
		Painter::drawSquare(hdc_, _pos, _size, color_); 
	}

	void move(int dx_, int dy_) {

		_pos.x += static_cast<LONG>(dx_ * _size);
		_pos.y += static_cast<LONG>(dy_ * _size);
	}

};

//enum class mycolor {
//	GREEN = RGB(0 255, 0);
//};

class Bait: GameObject {
	friend class Game;
	COLORREF _color = RGB(0, 255, 0);

public:
	void draw(const HDC hdc_) const { Painter::drawSquare(hdc_, _pos, _size, _color); }
	
	void reset() { 
		_color = RGB(0, 255, 0); 
	}
};


class Snake : GameObject
{
	friend class Game;
	std::vector<SnakeBody> _body;
	size_t _init_body_size = 3;
	POINT _direction{0, -1}; // init go up 
	size_t _speed = 100;

	void init(const int x_, const int y_) {
		if (_init_body_size == 0) { throw std::exception("init_size must be > 0"); }
		_body.emplace_back(x_, y_);
		grow(_init_body_size - 1);
		const auto& h = getHead();
		POINT bodyPos = h.getPos();
	}

	void clear_body() { _body.clear(); }
	
public:
	
	Snake(int x_ = 0, int y_ = 0) : GameObject(x_, y_) {  init(x_, y_); }
	Snake(const POINT pos_) : Snake(pos_.x, pos_.y) { }

	const	SnakeBody& getHead() const	{ return _body.front(); }
			SnakeBody& getHead()		{ return _body.front(); }
	
	const	SnakeBody& getTail() const	{ return _body.back(); }
			SnakeBody& getTail()		{ return _body.back(); }

	void reset(const POINT pos_) {
		GameObject::reset();
		clear_body();
		init(pos_.x, pos_.y);
	}

	size_t getSpeed() { return _speed; }

	void setSize() = delete;
	size_t getSize() const { return _body.size(); };


	void setDirection(const int x_, const int y_) {
		if (x_ == -_direction.x && y_ == -_direction.y) return; // no immidiate opposite
		_direction.x = x_;
		_direction.y = y_;
	}

	const POINT& getDirection() const { return _direction; }
	POINT& getDirection()  { return getDirection(); }

	POINT getNextPos() const {
		return getHead().getNextPos(_direction.x, _direction.y);
	}

	void move() {
		SnakeBody& h = getHead();
		
		// body follow
		for (size_t i = _body.size() - 1; i > 0; i--) {
			auto& dst = _body[i];
			const auto & src = _body[i - 1];
			dst.setPos(src.getPos());			
		}
		h.move(_direction.x, _direction.y);
	}

	void grow(const size_t n = 1) {
		for (int i = 0; i < n; i++) {
			_body.emplace_back(getTail().getPos());
		}
	}

	void draw(const HDC& hdc, const COLORREF& _headColor, const COLORREF& _bodyColor) const {
		for (int i = 1; i < _body.size(); i++) {
			const SnakeBody& b = _body[i];
			b.draw(hdc, _bodyColor); //draw body
		}

		getHead().draw(hdc, _headColor); //draw head, always on top;
		
	}

	void onKeyUp()		{ setDirection( 0, -1); }
	void onKeyDown()	{ setDirection( 0,  1); }
	void onKeyLeft()	{ setDirection(-1,  0); }
	void onKeyRight()	{ setDirection( 1,  0); }
};

class Game {
	//enum class GameState; // works?
	
	
	POINT _snake_init_pos{0, 0};
	Snake _snake;
	Bait _bait;
	HWND hWnd = NULL;
	bool _isPause = false;
	

	COLORREF _snakeHeadColor = RGB(255, 0, 0); // red head
	COLORREF _snakeBodyColor = RGB(128, 128, 128); //grey body
	
public:
	enum class GameState {
		None = 0,
		Landing,
		GamePlay,
		GameOver,
		LeaderBoard
	};
	
	GameState _currentState = GameState::Landing; // i want to put this to private but to declare the enum class in public.

	Game(int x_, int y_) : _snake(x_, y_), _snake_init_pos{ x_, y_ } { };
	Game() = default;
	Game(const POINT& pos_) : Game(pos_.x, pos_.y) { }

	void setCurrentState(GameState state) { _currentState = state; }
	GameState getCurrentState() const { return _currentState; }
	
	void setHwnd(HWND hWnd_) { hWnd = hWnd_;}
	Snake& getSnake() { return _snake; }

	void setHWnd(HWND hWnd_) { hWnd = hWnd_;}

	void restart(GameState dstGameState = GameState::Landing) {
		RECT cr;
		_currentState = dstGameState;
		GetClientRect(hWnd, &cr);
		_snake_init_pos.x = (cr.right - cr.left) / 2; 
		_snake_init_pos.y = (cr.bottom - cr.top) / 2; 

		if (!_snake.getSize()) return;
		int r = static_cast<int>(_snake.getHead().getSize());
		int rx = _snake_init_pos.x % r;
		int ry = _snake_init_pos.y % r;
		_snake_init_pos.x -= rx;
		_snake_init_pos.y -= ry;
		
		_snake.reset(_snake_init_pos);
		placeBait();
	}

	void update() {
		switch (_currentState)
		{
		case Game::GameState::None: break;
		case Game::GameState::Landing: break;
		case Game::GameState::GamePlay: { update_GamePlay(); } break;
		case Game::GameState::GameOver: break;
		case Game::GameState::LeaderBoard: break;
		default:
			break;
		}
		
	}

	void update_TitlePage() {
		if (_currentState != GameState::Landing) return;

	}

	void update_GamePlay() {
		if (_currentState != GameState::GamePlay) return;

		if (_isPause) {
			InvalidateRect(hWnd, nullptr, true);
			return;
		}

		_snake.move();
		
		if (isGameOver()) {
			_currentState = GameState::GameOver;
			InvalidateRect(hWnd, nullptr, true);
			return;
		}
		
		if (_snake.getHead().isCollided(_bait)) {
			_snake.grow(1);
			placeBait();
		}
		
		InvalidateRect(hWnd, nullptr, true);
	}

	bool isGameOver() const {
		
		const SnakeBody& h = _snake.getHead();
		for (int i = 1; i < _snake._body.size(); i++) {
			const SnakeBody& b = _snake._body[i];
			if (h.isCollided(b))  return true; 
		}


		RECT cr, temp, headRect;
		if (!GetClientRect(hWnd, &cr)) {
			auto lastError = GetLastError();
		}
		headRect = _snake.getHead().hitBox();

		return !IntersectRect(&temp, &cr, &headRect);
	}

	bool isValidBait(const Bait& bait_) const {
		for (const auto& sb : _snake._body) {
			if (bait_.isCollided(sb)) 
				return false;
		}
		return true;
	}

	void placeBaitWithType(Bait& bait_) {
		RECT cr;

		GetClientRect(hWnd, &cr);
		POINT randomPoint{ 0, 0 };

		cr.left += static_cast<int>(bait_._size);
		cr.top += static_cast<int>(bait_._size);
		cr.right -= static_cast<int>(bait_._size);
		cr.bottom -= static_cast<int>(bait_._size);
		int upperLimit = 1000;

		for (;;) {
			randomPoint = Util::getRandomPointInRect(cr);
			_bait.setPos(randomPoint);
			if (isValidBait(_bait)) break;
		}
	}
	
	void placeBait() { placeBaitWithType(_bait); }

	void draw(HDC hdc_) const {
		
		switch (_currentState)
		{		
			case GameState::None:			{ throw std::exception("GameState::None"); }	break;
			case GameState::Landing:		{ drawScene_Landing(hdc_); }							break;
			case GameState::GamePlay:		{ drawGamePlay(hdc_); }							break;
			case GameState::GameOver:		{ drawGameOver(hdc_); }							break;
			case GameState::LeaderBoard:	{ drawRankingBoard(hdc_); }						break;
			default:						{ throw std::exception("Unknown GameState"); }	break;
		}
	}

	void drawGamePlay(HDC hdc_) const {
		_snake.draw(hdc_, _snakeHeadColor, _snakeBodyColor);
		_bait.draw(hdc_);
		if (_isPause) { drawPause(hdc_); }
	}

	void togglePause() { _isPause = !_isPause; }
	
	void onEsc() {
		if (_currentState == GameState::GameOver) {
			restart(GameState::GamePlay);
		}
	}
	
	void onSpace() {
		switch (_currentState)
		{
			case Game::GameState::Landing:		{ setCurrentState(GameState::GamePlay); } break;
			case Game::GameState::GamePlay:		{ togglePause(); } break;
			case Game::GameState::GameOver:		{ setCurrentState(GameState::LeaderBoard); } break;
			case Game::GameState::LeaderBoard:	{ setCurrentState(GameState::Landing); } break;
			case Game::GameState::None:			{ assert(false && "GameState::None"); } break;
			default:							{ assert(false && "Unknow GameState"); } break;
		}
	}

	
	void drawPause(HDC hdc_) const {
		RECT cr;
		GetClientRect(hWnd, &cr);
		Painter::drawMessage(hWnd, hdc_, L"Pause", cr, RGB(127, 127, 127), RGB(0, 0, 0));
	}



	void drawScene_Landing(HDC hdc_) const {
		// todo: draw a `snake` title 
		Painter::drawScene_Landing(hWnd, hdc_);
		RECT cr;
		GetClientRect(hWnd, &cr);
		// drawSprite
		cr.top += (cr.bottom - cr.top) / 2;
		Painter::drawMessage(hWnd, hdc_, L"Press <SPACE> To Continue", cr, RGB(127, 127, 127), RGB(0, 0, 0));
		printf("drawTitle");
	}

	void drawGameOver(HDC hdc_) const {
		// todo: draw a game over in the middle of the screen
		drawGamePlay(hdc_);
		
		RECT cr;
		GetClientRect(hWnd, &cr);
		Painter::drawMessage(hWnd, hdc_, L"Game Over", cr, RGB(255, 0, 0), RGB(255, 255, 255));
		cr.top += (cr.bottom - cr.top) / 3;
		Painter::drawMessage(hWnd, hdc_, L"Press <SPACE> To continue", cr, RGB(127, 127, 127), RGB(0, 0, 0));
		cr.top += (cr.bottom - cr.top) / 5;
		Painter::drawMessage(hWnd, hdc_, L"Press <ESC> To restart", cr, RGB(127, 127, 127), RGB(0, 0, 0));
		
	}

	void drawRankingBoard(HDC hdc_) const {
		// todo: draw a ranking board 
		// 
		// 1. backToTitle 2. quitGame
		printf("drawLeaderBoard");
	}
};