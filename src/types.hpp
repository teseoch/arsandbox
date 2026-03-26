#pragma once

struct Vec2
{
	float x = 0, y = 0;
	Vec2() = default;
	Vec2(float x_, float y_) : x(x_), y(y_) {}
};
struct Quad
{
	Vec2 v[4];
};