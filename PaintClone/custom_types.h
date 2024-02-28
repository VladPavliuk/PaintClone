#pragma once

#include <cassert>

struct int2
{
	int x;
	int y;

	int2 operator+(int2 a)
	{
		return { a.x + x, a.y + y };
	}

	int2& operator+=(int2 const& a)
	{
		x += a.x;
		y += a.y;
		return *this;
	}

	int2 operator-(int2 a)
	{
		return { x - a.x, y - a.y };
	}

	int2& operator-=(int2 const& a)
	{
		x -= a.x;
		y -= a.y;
		return *this;
	}

	int2 operator*(int a)
	{
		return { a * x, a * y };
	}

	int2& operator*=(int const& a)
	{
		x *= a;
		y *= a;
		return *this;
	}

	int2& operator/=(int const& a)
	{
		x /= a;
		y /= a;
		return *this;
	}
};

struct float2
{
	float x;
	float y;
};

struct char3
{
	char x;
	char y;
	char z;
};

struct int3
{
	int x;
	int y;
	int z;
};

typedef unsigned char ubyte;

struct ubyte3
{
	ubyte x;
	ubyte y;
	ubyte z;

	bool operator==(ubyte3 const& b)
	{
		return x == b.x && y == b.y && z == b.z;
	}
};

struct ubyte4
{
	ubyte4()
	{}

	ubyte4(ubyte x, ubyte y, ubyte z, ubyte w) :
		x(x), y(y), z(z), w(w)
	{}

	ubyte x;
	ubyte y;
	ubyte z;
	ubyte w;

	ubyte4& operator+=(ubyte4 const& b)
	{
		x += b.x;
		y += b.y;
		z += b.z;
		w += b.w;
		return *this;
		//return { (ubyte)(x + b.x), (ubyte)(y + b.y), (ubyte)(z + b.z), (ubyte)(w + b.w) };
	}

	bool operator==(ubyte4 const& b)
	{
		return x == b.x && y == b.y && z == b.z && w == b.w;
	}

	bool operator!=(ubyte4 const& b)
	{
		return !(x == b.x && y == b.y && z == b.z && w == b.w);
	}
};

struct ubyte2
{
	ubyte x;
	ubyte y;
};

typedef unsigned short ushort;
typedef unsigned int uint;

struct float4
{
	float x;
	float y;
	float z;
	float w;
};

struct int4
{
	int x;
	int y;
	int z;
	int w;

	int2 xy()
	{
		return { x, y };
	}

	void xy(int2 a)
	{
		x = a.x;
		y = a.y;
	}

	void zw(int2 a)
	{
		z = a.x;
		w = a.y;
	}

	int2 zw()
	{
		return { z, w };
	}

	int2 size()
	{
		return { z - x, w - y };
	}

	int4 operator*(float a)
	{
		return { (int)((float)x * a), (int)((float)y * a), (int)((float)z * a), (int)((float)w * a) };
	}

	int4& operator/=(float const& a)
	{
		x = (int)((float)x / a);
		y = (int)((float)y / a);;
		z = (int)((float)z / a);;
		w = (int)((float)w / a);;
		return *this;
	}
};

//struct Rect
//{
//	int x;
//	int y;
//	int z;
//	int w;
//
//	int2 size;
//
//	void UpdateSize()
//	{
//		size.x = z - x;
//		size.y = w - y;
//
//		assert(size.x >= 0);
//		assert(size.y >= 0);
//	}
//
//	void UpdateTopRight()
//	{
//		z = x + size.x;
//		w = y + size.y;
//	}
//
//	int4 asInt4()
//	{
//		return { x, y, z, w };
//	}
//};
