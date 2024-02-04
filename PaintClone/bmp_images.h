#pragma once

#include "files.h"

#include <memory.h>

// Should use pack command to avoid the default 4 bytes packing since we use 2 bytes members here
// to map directly from the binary file
#pragma pack(push,2)
struct BmpFileHeader
{
	ushort signature;
	uint fileSize;
	ushort reserved1;
	ushort reserved2;
	uint offsetToBitmap;
};
#pragma pack(pop)

struct BmpDIBHeader
{
	uint dibHeaderSize;
	int imageWidth;
	int imageHeight;
	ushort planes; // must be 1
	ushort bitsPerPixel;
	uint compression; // 0 means no compression
	uint imageSize; // if compression is 0, it might be 0 as well (BUT NOT ALWAYS!)
	uint xPixelsPerMeter;
	uint yPixelsPerMeter;
	uint colorsInColorTable;
	uint importantColorsCount;
};

struct BmpImage
{
	ubyte4* rgbaBitmap;
	int2 size;
};

BmpImage LoadBmpFile(const wchar_t* fileName);
