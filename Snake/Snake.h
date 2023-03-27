#pragma once

#include "resource.h"
#include "Resource.h"
#include <vector>
#include <cassert>
#include <stdexcept>
#include <string>
#include <time.h>


enum class Direction { N = 0,  E,  S, W	 };

class MyBitMap {
	int _id = 0;
	HBITMAP _bitmap = NULL;
	BITMAP _info;

public:
	MyBitMap() = delete;
	MyBitMap(int id_) : _id(id_) { init(_id); };

	void init(int id) {
		loadFromResource(id);
		int result = GetObject(_bitmap, sizeof(_info), &_info);
		if (result <= 0) {
			auto e = GetLastError();
	
		}
	}

	void loadFromResource(int id) {
		_bitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(id));
	}

	const BITMAP&	info()    const { return _info;          }
	const HBITMAP&	bitmap()  const { return _bitmap;		 }
	const LONG&	    width()	  const { return _info.bmWidth;  }
	const LONG&	    height()  const { return _info.bmHeight; }

	int id() const { return _id; }

	void draw(HDC hdc_, HDC srcDC, int x_, int y_) const {
		if (!_bitmap) { assert(false); return; }
		if (!srcDC) { assert(false); return; }
	
		SelectObject(srcDC, _bitmap);
		
		int is_ok = BitBlt(hdc_, x_, y_, (int)width(), (int)height(), srcDC, 0, 0, SRCCOPY);
		if (!is_ok) { 
			int e = GetLastError(); 
			return;
		}
	}
};

class Sprite {
	int _currentFrameIdx = 0;
	int _beginFrameId = 0;
	int _endFrameId = 0;
	std::vector<MyBitMap> _frames;

public:
	Sprite() = default;
	Sprite(int beginFrameId_, int endFrameId_) : _beginFrameId(beginFrameId_), _endFrameId(endFrameId_) { init(); };

	void init() {
		for (int id = _beginFrameId; id < _endFrameId + 1; id++) {
			_frames.emplace_back(id);
		}
	}

	int nFrames() const { return (int)_frames.size(); }

	const MyBitMap& currentFrame() const {
		if (_frames.size() == 0) assert(false);
		return _frames[_currentFrameIdx];
	}

	const LONG& width() const {
		return currentFrame().width();
	}

	const LONG& height() const {
		return currentFrame().height();
	}

	const int currentFrameIdx() const { return _currentFrameIdx; }

	void draw(HDC hdc_, HDC srcDC_, int x_, int y_) const {
		currentFrame().draw(hdc_, srcDC_, x_, y_);
	}

	void nextFrame() {
		_currentFrameIdx = (_currentFrameIdx + 1) % nFrames();
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

	static void drawTitle(HWND hWnd_, HDC hdc_, HDC srcDC_, const Sprite& sprite_) {
		RECT tmpCrRect;
		RECT cr;
		GetClientRect(hWnd_, &cr);
		tmpCrRect = cr;
		
		
		LONG cr_w = cr.right - cr.left;
		LONG cr_h = cr.bottom - cr.top;
		POINT spos;
		POINT cr_mid;
		cr_mid.x = cr.left + cr_w / 2;
		cr_mid.y = cr.top + cr_h / 2;
		spos.x = cr_mid.x - sprite_.width()  / 2;
		spos.y = cr_mid.y - sprite_.height() / 2 ;
		
		bool b = sprite_.currentFrameIdx() == sprite_.nFrames() - 1;
		Rectangle(hdc_, spos.x - 5, spos.y - 5, spos.x + sprite_.width() + 5, spos.y + sprite_.height() + 5);	
		sprite_.draw(hdc_, srcDC_, spos.x, spos.y);
		
		cr = tmpCrRect;
		cr.bottom -= cr_h / 2;
		Painter::drawMessage(hWnd_, hdc_, L"SNAKE", cr, TRANSPARENT, RGB(255, 255, 255), 48); //TRANSPARENT is black?

		cr = tmpCrRect;
		cr.top += (cr.bottom - cr.top) / 2;
		Painter::drawMessage(hWnd_, hdc_, L"Press <SPACE> To Play", cr, RGB(127, 127, 127), RGB(0, 0, 0));

	}

	static void drawScene_Ranking(HWND hWnd_, HDC hdc_) {
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
		DrawText(hdc_, TEXT("Ranking here"), 5, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DeleteObject(tPen);
	}

	static void drawMessage(HWND hWnd_, HDC hdc_, const wchar_t* s, RECT rect_, COLORREF bgColor = TRANSPARENT, COLORREF textColor = RGB(0, 0, 0), int fontSize = NULL) {
		SetTextColor(hdc_, textColor);
		SetBkColor(hdc_, bgColor);
		if (fontSize) {
			int savedDC = SaveDC(hdc_);
			HFONT hf = CreateFont(/*size=*/-fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Sans Serif");
			SelectObject(hdc_, hf);
			DrawText(hdc_, s, (int)wcslen(s), &rect_, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			DeleteObject(hf);
			RestoreDC(hdc_, savedDC);
		}
		else {
			DrawText(hdc_, s, (int)wcslen(s), &rect_, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
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
	Direction _currentDirection = Direction::N;
	size_t _speed = 100;
	bool _canSetDirection = false;

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

	const SnakeBody& getHead() const	{ return _body.front(); }
	const SnakeBody& getTail() const	{ return _body.back();  }

	void reset(const POINT pos_) {
		GameObject::reset();
		clear_body();
		init(pos_.x, pos_.y);
		_canSetDirection = false;
	}

	size_t getSpeed() { return _speed; }

	void setSize() = delete;
	size_t getSize() const { return _body.size(); };

	bool isOppositeDirection(Direction d) const {
		switch (d)
		{
			case Direction::N: { return _currentDirection == Direction::S; } break;
			case Direction::E: { return _currentDirection == Direction::W; } break;
			case Direction::S: { return _currentDirection == Direction::N; } break; 
			case Direction::W: { return _currentDirection == Direction::E; } break;
			default: { assert(false && "you should not be here."); } break;
		}
		return false;
	}


	void setDirection(Direction d) {
		if (!_canSetDirection || isOppositeDirection(d)) return;
		_currentDirection = d;
		_canSetDirection = false; // wait move() to release;
		
	}

	const Direction& getCurrentDirection() const { return _currentDirection; }
	Direction& getCurrentDirection()  { return getCurrentDirection(); }

	POINT getCurrentDirectionAsVector() const {
		POINT cd;
		switch (_currentDirection)
		{
			case Direction::N: { cd.x = 0; cd.y = -1; } break;
			case Direction::E: { cd.x = 1; cd.y = 0; } break;
			case Direction::S: { cd.x = 0; cd.y = 1; } break;
			case Direction::W: { cd.x = -1; cd.y = 0; } break;
			default: { assert(false && "you should not be here"); } break;
		}
		return cd;
	}

	POINT getPos() const { return getHead().getPos(); }

	POINT getNextPos() const {
		POINT cd = getCurrentDirectionAsVector();
		return getHead().getNextPos(cd.x, cd.y);
	}

	void move() {
		SnakeBody& h = _body.front();
		
		// body follow
		for (size_t i = _body.size() - 1; i > 0; i--) {
			auto& dst = _body[i];
			const auto & src = _body[i - 1];
			dst.setPos(src.getPos());			
		}

		POINT cd = getCurrentDirectionAsVector();
		h.move(cd.x, cd.y);
		_canSetDirection = true;
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

	void onKeyUp()		{ setDirection(Direction::N); }
	void onKeyRight()	{ setDirection(Direction::E); }
	void onKeyDown()	{ setDirection(Direction::S); }
	void onKeyLeft()	{ setDirection(Direction::W); }
};

enum class GameState {
	None = 0,
	Landing,
	GamePlay,
	GameOver,
	Ranking
};

struct LayoutBox {
	std::string name = "";
	POINT pos{ 0, 0 }; //topleft corner
	int width = 0;
	int height = 0;

	LayoutBox() = default;
	LayoutBox(const char* name_, int width_, int height_) : name(name_), width(width_), height(height_) { }

	POINT midPoint() {
		POINT p;
		p.x = (pos.x + width) / 2;
		p.y = (pos.y + height) / 2;
		return p;
	}

	RECT rect() const {
		RECT r;
		r.left = pos.x;
		r.right = pos.x + width;
		r.top = pos.y;
		r.bottom = pos.y + height;
		return r;
	}

	LayoutBox hCombine(LayoutBox r) {
		r.pos.x = pos.x + height; // to right
		
		LayoutBox lr;
		lr.pos = pos;
		lr.width = r.width + width;
		lr.height = max(r.height, height);
		return lr;
	}

	LayoutBox vCombine(LayoutBox& r) {
		r.pos.y = pos.y + height; // to below
		
		LayoutBox lr;
		lr.pos = pos;
		lr.width = max(r.width, width);
		lr.height = r.height + height;
		return lr;
	}

	void draw(HDC hdc_) const {
		RECT r = rect();
		HPEN pen = (HPEN) GetStockObject(DC_PEN);
		int savedDc =  SaveDC(hdc_);
		
		SetDCPenColor(hdc_, RGB(127, 127, 127));
		SelectObject(hdc_, pen);
		SelectObject(hdc_, GetStockObject(NULL_BRUSH));
		Rectangle(hdc_, r.left, r.top, r.right, r.bottom);
		
		RestoreDC(hdc_, savedDc);
	}
};

struct GameLayout {
	LayoutBox clientRect;
	LayoutBox uiRect;
	LayoutBox gameRect;

	void initGameRect(int cell_size = 30, int n_cells_per_side = 20) {
		gameRect.width = cell_size * n_cells_per_side;
		gameRect.height = gameRect.width;
	}

	void initUiRect(int height) {
		uiRect.width = gameRect.width;
		uiRect.height = height;
	}

	void init(int cell_size = 30, int n_cells_per_side = 20, DWORD wnd_style = WS_OVERLAPPEDWINDOW) {
		initGameRect(cell_size, n_cells_per_side);
		int ui_h = cell_size;
		initUiRect(ui_h);
		clientRect = uiRect.vCombine(gameRect);
		RECT cr = clientRect.rect();
		AdjustWindowRect(&cr, wnd_style, false);
		clientRect.width = cr.right - cr.left;
		clientRect.height = cr.bottom - cr.top;
	}

	void draw(HDC hdc_) const {
		uiRect.draw(hdc_);
		gameRect.draw(hdc_);
	}

	RECT getGameRect() const { return gameRect.rect(); };
	RECT getUiRect() const { return uiRect.rect(); };
	RECT rect() const { return clientRect.rect(); };

};

class Game {

private:
	POINT _snake_init_pos{0, 0};
	Snake _snake;
	Bait _bait;
	HWND hWnd = NULL;
	HDC srcDC = NULL;
	bool _isPause = false;
	
	Sprite _landingSprite;
	COLORREF _snakeHeadColor = RGB(255, 0, 0); // red head
	COLORREF _snakeBodyColor = RGB(128, 128, 128); //grey body
	GameState _currentState = GameState::Landing; 
	
public:
	//Game() = default;
	Game(int x_ = 0, int y_ = 0) : _snake(x_, y_), _snake_init_pos{ x_, y_ }, _landingSprite(IDB_BITMAP1, IDB_BITMAP13) { srand((unsigned int) time(NULL)); };
	Game(const POINT& pos_) : Game(pos_.x, pos_.y) { }
	~Game() { if (srcDC) { DeleteDC(srcDC); } }
	GameLayout gameLayout;
	
	void setCurrentState(GameState state) { _currentState = state; }
	GameState getCurrentState() const { return _currentState; }
	
	void setHwnd(HWND hWnd_) { hWnd = hWnd_;}
	Snake& getSnake() { return _snake; }


	void init(HWND hWnd_) { 
		hWnd = hWnd_;
		HDC tmpDC = GetDC(hWnd_);
		srcDC = CreateCompatibleDC(tmpDC);
		ReleaseDC(hWnd, tmpDC);
		//gameLayout.init(hWnd, (int)_bait.getSize(), 20);
	}



	RECT gameRect() const { return gameLayout.getGameRect(); }
	RECT uiRect() const { return gameLayout.getUiRect(); }

	void restart(GameState dstGameState = GameState::Landing) {
		RECT gr = gameRect();
		_snake_init_pos.x = (gr.right - gr.left) / 2; 
		_snake_init_pos.y = (gr.bottom - gr.top) / 2; 

 		if (!_snake.getSize()) return;
		_snake.reset(adjustedPosition(_snake_init_pos, _snake.getHead()));
		placeBait();
		setCurrentState(dstGameState);
	}

	void update() {
		switch (_currentState)
		{
			case GameState::None: break;
			case GameState::Landing: {update_Landing(); }break;
			case GameState::GamePlay: { update_GamePlay(); } break;
			case GameState::GameOver: break;
			case GameState::Ranking: break;
			default:
				break;
		}
	}

	void update_Landing() {
		_landingSprite.nextFrame();
		InvalidateRect(hWnd, nullptr, true);
	}

	void update_GamePlay() {
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
			if (h.isCollided(b)) return true; 
		}
		RECT gr = gameRect();

		if (_snake._currentDirection == Direction::N || _snake._currentDirection == Direction::W)
			return !PtInRect(&gr, _snake.getPos());
		return !PtInRect(&gr, _snake.getNextPos());
	}

	bool isValidBait(const Bait& bait_) const {
		POINT rb_pt{
			bait_.hitBox().right,
			bait_.hitBox().bottom
		};
		
		RECT gr = gameRect();
		
		if (!PtInRect(&gr, rb_pt)) 
			return false;

		for (const auto& sb : _snake._body) {
			if (bait_.isCollided(sb)) 
				return false;
		}
		return true;
	}

	void placeBaitWithType(Bait& bait_) {
		RECT gr = gameRect();
		int upperLimit = 1000;
		POINT randomPoint{ 0, 0 };
		
		
		for (int c = 0; c < upperLimit; c++) {
			randomPoint = Util::getRandomPointInRect(gr);
			randomPoint = adjustedPosition(randomPoint, _bait);
			_bait.setPos(randomPoint);
			if (isValidBait(_bait)) return;
		}
		assert(false && "sth is wrong");
	}

	POINT adjustedPosition(POINT pos, const GameObject& o) const {
		int r = static_cast<int>(o.getSize());
		int rx = pos.x % r;
		int ry = pos.y % r;
		pos.x -= rx;
		pos.y -= ry;
		return pos;
	}
	
	void placeBait() { placeBaitWithType(_bait); }

	void draw(HDC hdc_) const {
		
		switch (_currentState)
		{		
			case GameState::None:			{ throw std::exception("GameState::None"); }	break;
			case GameState::Landing:		{ drawTitle(hdc_); }							break;
			case GameState::GamePlay:		{ drawGamePlay(hdc_); }							break;
			case GameState::GameOver:		{ drawGameOver(hdc_); }							break;
			case GameState::Ranking:		{ drawRanking(hdc_); }							break;
			default:						{ throw std::exception("Unknown GameState"); }	break;
		}
	}

	void drawGamePlay(HDC hdc_) const {
		gameLayout.draw(hdc_);
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
			case GameState::Landing:		{ setCurrentState(GameState::GamePlay); }	break;
			case GameState::GamePlay:		{ togglePause();						}   break;
			case GameState::GameOver:{ 
				setCurrentState(GameState::Ranking); 
				InvalidateRect(hWnd, nullptr, true);	
			} break;
			case GameState::Ranking:		{ restart();							}	break;
			case GameState::None:			{ assert(false && "GameState::None");	}	break;
			default:						{ assert(false && "Unknow GameState");	}	break;
		}
	}

	
	void drawPause(HDC hdc_) const {
		RECT cr;
		GetClientRect(hWnd, &cr);
		Painter::drawMessage(hWnd, hdc_, L"Pause", cr, RGB(127, 127, 127), RGB(0, 0, 0));
		cr.top += (cr.bottom - cr.top) / 4;
		Painter::drawMessage(hWnd, hdc_, L"Press <SPACE> to resume", cr, RGB(127, 127, 127), RGB(0, 0, 0));
	}

	void drawTitle(HDC hdc_) const { Painter::drawTitle(hWnd, hdc_, srcDC, _landingSprite); }

	void drawGameOver(HDC hdc_) const {
		drawGamePlay(hdc_);
		
		RECT cr;
		GetClientRect(hWnd, &cr);
		Painter::drawMessage(hWnd, hdc_, L"Game Over", cr, RGB(255, 0, 0), RGB(255, 255, 255));
		cr.top += (cr.bottom - cr.top) / 3;
		Painter::drawMessage(hWnd, hdc_, L"Press <SPACE> To continue", cr, RGB(127, 127, 127), RGB(0, 0, 0));
		cr.top += (cr.bottom - cr.top) / 5;
		Painter::drawMessage(hWnd, hdc_, L"Press <ESC> To restart", cr, RGB(127, 127, 127), RGB(0, 0, 0));
		
	}

	void drawRanking(HDC hdc_) const {
		
		RECT cr;
		GetClientRect(hWnd, &cr);
		Painter::drawMessage(hWnd, hdc_, L"Ranking here.", cr, RGB(127, 127, 127), RGB(0, 0, 0));
	}
};