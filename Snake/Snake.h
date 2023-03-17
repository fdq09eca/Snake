#pragma once

#include "resource.h"
#include <vector>

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
	size_t _size = 0;

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
	void reset() { 
		resetPos();
		resetSize();
	};
	
	const POINT& getPos() const { return _pos; }
	POINT getPos() { return _pos; }
	
	void setPos(const int x_, const int y_) { _pos.x = x_; _pos.y = y_; }
	void setPos(const POINT pos_) { setPos(pos_.x, pos_.y); }

	const size_t& getSize() const { return _size; };
	size_t getSize() { return _size; };
	void setSize(const size_t size) { _size = size; };

	const RECT hitBox() const {
		RECT r{ _pos.x, _pos.y, _pos.x + _size, _pos.y + _size};
		return r;
	}

	bool isCollided(const GameObject& b) {
		RECT t;
		RECT aBox = hitBox();
		RECT bBox = b.hitBox();
		return IntersectRect(&t, &aBox, &bBox) > 0;
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

	void move(const int x_, const int y_) {
		_pos.x += x_ * _size;
		_pos.y += y_ * _size;
	}

};

class Bait: GameObject {
	friend class Game;
	bool  _is_placed = false;
	COLORREF _color = RGB(0, 255, 0);


public:
	void draw(const HDC& hdc_) const { Painter::drawSquare(hdc_, _pos, _size, _color); }
};


class Snake:GameObject
{
	std::vector<SnakeBody> _body;

public:
	Snake(const int x_, const int y_, const size_t init_size = 3) : GameObject(x_, y_) { 
		if (init_size == 0) { throw std::exception("init_size must be > 0"); }
		_body.emplace_back(x_, y_); 
		grow(init_size - 1);
	}
	
	Snake(const POINT pos_, size_t init_size = 3) : Snake(pos_.x, pos_.y, init_size) { }

	
	const SnakeBody& getHead() const { return _body.front(); }
	SnakeBody& getHead() { return getHead(); }
	
	const SnakeBody& getTail() const { return _body.back(); }
	SnakeBody& getTail() { return getTail(); }

	

	void move(const int x, const int y) {
		SnakeBody& h = getHead();
		POINT prevPos = h.getPos();
		h.move(x, y);
		
		// body follow
		for (int i = 0; i < _body.size(); i++) {
			SnakeBody& b = _body[i];
			prevPos = b.getPos();
			b.setPos(prevPos);
		}
	}

	

	void grow(const size_t n = 1) {
		for (int i = 0; i < n; i++) {
			_body.emplace_back(_body.back().getPos());
		}
	}

	void draw(const HDC& hdc, const COLORREF& _headColor, const COLORREF& _bodyColor) const {
		for (int i = 1; i < _body.size(); i++) {
			const SnakeBody& b = _body[i];
			b.draw(hdc, _bodyColor); //draw body
		}
		getHead().draw(hdc, _headColor); // draw head
	}
};

class Game {
	Snake _snake;
	Bait _bait;
	

	COLORREF _snakeHeadColor = RGB(255, 0, 0); // red head
	COLORREF _snakeBodyColor = RGB(128, 128, 128); //grey body
public:
	Game() : Game(0, 0) {};
	Game(const POINT pos_) : Game(pos_.x, pos_.y) { }
	Game(const int x_, const int y_) :_snake(x_, y_) {
		placeBait(); 
	}

	

	void placeBait() {
		// place it randomly;
		_bait.setPos(200, 200);
	}

	void draw(const HDC& hdc_) const {
		_snake.draw(hdc_, _snakeHeadColor, _snakeBodyColor);
		_bait.draw(hdc_);
	}
};