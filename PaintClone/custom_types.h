#pragma once

struct int2
{
	int x;
	int y;
};

struct char3
{
	char x;
	char y;
	char z;
};

typedef unsigned char ubyte;

struct ubyte3
{
	ubyte x;
	ubyte y;
	ubyte z;
};

struct ubyte4
{
	ubyte4()
	{}

	ubyte4(ubyte x, ubyte y, ubyte z, ubyte w):
		x(x), y(y), z(z), w(w)
	{}

	ubyte x;
	ubyte y;
	ubyte z;
	ubyte w;
};

struct ubyte2
{
	ubyte x;
	ubyte y;
};
