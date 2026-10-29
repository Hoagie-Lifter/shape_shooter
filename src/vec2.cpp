#include <math.h>
#include "vec2.h"

// Vec2:: namespace prefix required because declarations in separate file
// Vec2() would be free function without without namespace prefix, rather than a member function of Vec2 class

Vec2::Vec2()
{
	
}

Vec2::Vec2(float xin, float yin)
	: x(xin), y(yin)
{
	
}

Vec2 Vec2::operator + (const Vec2 & rhs) const
{
	return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator - (const Vec2 & rhs) const
{
	return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator * (const float val) const
{
	return Vec2(x * val, y * val);
}

Vec2 Vec2::operator / (const float val) const
{
	return Vec2(x / val, y / val);
}

bool Vec2::operator == (const Vec2 & rhs) const
{
	return (x == rhs.x && y == rhs.y);
}

bool Vec2::operator != (const Vec2 & rhs) const
{
	return (x != rhs.x && y != rhs.y);;
}

void Vec2::operator += (const Vec2 & rhs)
{
	x += rhs.x;
	y += rhs.y;
}

void Vec2::operator -= (const Vec2 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void Vec2::operator *= (const float val)
{
	x *= val;
	y *= val;
}

void Vec2::operator /= (const float val)
{
	x /= val;
	y /= val;
}

float Vec2::dist(const Vec2 & rhs) const
{
	Vec2 lhs {x, y};
	lhs -= rhs;
	
	return std::sqrt(lhs.x * lhs.x + lhs.y * lhs.y);
}

Vec2 Vec2::dir(const Vec2 & rhs) const
{
	Vec2 lhs {x, y};
	lhs -= rhs;
	
	float magnitude = std::sqrt(lhs.x * lhs.x + lhs.y * lhs.y);
	if (magnitude != 0)
	{
		lhs /= magnitude;
	}
	
	return lhs;
}