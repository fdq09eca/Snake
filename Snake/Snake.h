#pragma once

#include "resource.h"
#include <vector>

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
	static void drawSquare(const HDC& hdc_, const POINT& pos_, const size_t& size_, const COLORREF& color)
	{
		SelectObject(hdc_, GetStockObject(DC_BRUSH));
		SetDCBrushColor(hdc_, color);
		Rectangle(hdc_, pos_.x, pos_.y, pos_.x + size_, pos_.y + size_);
	}

};


class GameObject {

protected:
	POINT _pos{ 0, 0 };
	size_t _size = 30;

public:

	GameObject() : GameObject(0, 0) {}
	GameObject(const int x, const int y, const size_t size_ = 30)
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
	const POINT getNextPos(const int x, const int y) const {
		POINT p = getPos();
		p.x += x;
		p.y += y;
		return  p;
	}
	
	void setPos(const int x_, const int y_) { _pos.x = x_; _pos.y = y_; }
	void setPos(const POINT pos_) { setPos(pos_.x, pos_.y); }

	const size_t& getSize() const { return _size; };
	size_t getSize() { return _size; };
	void setSize(const size_t size) { _size = size; };

	const RECT hitBox() const {
		RECT r{ _pos.x, _pos.y, _pos.x + _size, _pos.y + _size};
		return r;
	}

	const bool isCollided(const GameObject& other) const {
		RECT t;
		RECT thisBox = hitBox();
		RECT otherBox = other.hitBox();
		return IntersectRect(&t, &thisBox, &otherBox) > 0;
	}
};

class SnakeBody : GameObject {
	friend class Snake;
	friend class Game;

public:	
	SnakeBody(const int x_, const int y_, const size_t size_ = 30) : GameObject(x_, y_, size_) { }
	SnakeBody(const POINT pos_, const size_t size_ = 30) : SnakeBody(pos_.x, pos_.y, size_) {}

	~SnakeBody(){	
		printf("SnakeBody dtor: pos[x: %d, y: %d]", _pos.x, _pos.y);
		reset();
	}
	
	
	void draw(const HDC& hdc_, const COLORREF& color_) const { 
		Painter::drawSquare(hdc_, _pos, _size, color_); 
	}

	void move(const int dx_, const int dy_) {
		_pos.x += dx_ * _size;
		_pos.y += dy_ * _size;
	}

};

class Bait: GameObject {
	friend class Game;
	COLORREF _color = RGB(0, 255, 0);

public:
	void draw(const HDC& hdc_) const { Painter::drawSquare(hdc_, _pos, _size, _color); }
	
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

	void init(const int x_, const int y_) {
		if (_init_body_size == 0) { throw std::exception("init_size must be > 0"); }
		_body.emplace_back(x_, y_);
		grow(_init_body_size - 1);
		auto h = getHead();
		POINT bodyPos = h.getPos();
		
		/*for (int i = 1; i < getSize(); i++) {
			auto dy = h.getSize() * i;
			bodyPos.y = dy;
			_body[i].setPos(bodyPos);
		}*/
	}

	void clear_body() { _body.clear(); }
	
public:
	Snake(const int x_, const int y_) : GameObject(x_, y_) {  init(x_, y_); }
	Snake(const POINT pos_) : Snake(pos_.x, pos_.y) { }

	const SnakeBody& getHead() const { return _body.front(); }
	SnakeBody& getHead() { return _body.front(); }
	
	const SnakeBody& getTail() const { return _body.back(); }
	SnakeBody& getTail() { return _body.back(); }

	void reset(const POINT pos_) {
		GameObject::reset();
		clear_body();
		init(pos_.x, pos_.y);
	}

	void setSize() = delete;
	const size_t& getSize() const { return _body.size(); };
	size_t getSize() { return _body.size(); };
	

	void setDirection(const int x_, const int y_) {
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
		for (int i = _body.size() - 1; i > 0; i--) {
			auto& r = _body[i];
			auto & l = _body[i - 1];
			r.setPos(l.getPos());			
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

	void onKeyUp() { setDirection(0, -1); }
	void onKeyDown() { setDirection(0, 1); }
	void onKeyLeft() { setDirection(-1, 0); }
	void onKeyRight() { setDirection(1, 0); }
};

class Game {
	Snake _snake;
	Bait _bait;
	POINT _snake_init_pos;
	HWND hWnd;

	COLORREF _snakeHeadColor = RGB(255, 0, 0); // red head
	COLORREF _snakeBodyColor = RGB(128, 128, 128); //grey body
	
public:
	Game(const int x_, const int y_) : _snake(x_, y_), _snake_init_pos{ x_, y_ } {}
	Game() : Game(0, 0) {};
	Game(const POINT pos_) : Game(pos_.x, pos_.y) { }
	Game(const HWND& hWnd_, const int x_, const int y_) : Game(x_, y_) {
		hWnd = hWnd_;
		placeBait();
	};

	Snake& getSnake() { return _snake; }

	void restart() {
		_snake.reset(_snake_init_pos);
		_bait.reset();
		//placeBait(); << should be this.
	}

	void update() {
		if (_snake.isCollided(_bait)) {
			_snake.grow(1);
			placeBait();
		}
		InvalidateRect(hWnd, nullptr, true);
	}

	const bool isGameOver() const {
		const SnakeBody& h = _snake.getHead();
		
		for (int i = 1; i < _snake._body.size(); i++) {
			const SnakeBody& b = _snake._body[i];
			if (h.isCollided(b)) return true;
		}


		RECT cr, temp, headRect;
		if (!GetClientRect(hWnd, &cr)) {
			auto lastError = GetLastError();
		}
		headRect = _snake.getHead().hitBox();


		return !IntersectRect(&temp, &cr, &headRect);
	}

	const bool isValidBait(const Bait& bait_) const {
		for (const auto& sb : _snake._body) {
			if (bait_.isCollided(sb)) 
				return false;
		}
		return true;
	}

	void placeBait(Bait& bait_) {
		RECT cr;

		GetClientRect(hWnd, &cr);
		POINT randomPoint{ 0, 0 };
		
		cr.left += bait_._size;
		cr.top += bait_._size;
		cr.right -= bait_._size;
		cr.bottom -= bait_._size;
		
		while (true) {
			randomPoint = Util::getRandomPointInRect(cr);
			_bait.setPos(randomPoint);
			if (isValidBait(_bait)) break;
		}
	}
	
	void placeBait() {
		placeBait(_bait);
	}

	void draw(const HDC& hdc_) const {
		_snake.draw(hdc_, _snakeHeadColor, _snakeBodyColor);
		_bait.draw(hdc_);
	}
};