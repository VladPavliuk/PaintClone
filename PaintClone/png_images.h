#pragma once

#include "files.h"

#include <memory.h>
#include <intrin.h>
#include <corecrt_wstring.h>
#include "hash_table.h"

// PNG general description
//
// FILTERING
// each scanline has it's own filtering
// the selected scanline filter is applied to each chanel (RGB) independently
// 5 Filters:
// 1. No filter
// 2. Difference between X and A
// 3. Difference between X and B
// 4. Difference between X and (A + B)/2 (average)
// 5. Paeth predictor (linear function of A,B,C)
//
// COMPRESSION
// For PNG DEFLATE algorithm is used (combination of LZ77 coding and Huffman coder)
// 
#pragma pack(push,1)
struct PngIHDRChunk
{
	uint length;
	uint signature;
	uint width;
	uint height;
	ubyte bitDepth;
	ubyte colorType;
	ubyte compressionMethod; // 0 is always used and means deflate/inflate compression
	ubyte filterMethod; // 0 is always used and means adaptive filtering with five basic filter types
	ubyte interlaceMethod;
	uint crc;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PngIDATChunk
{
	uint length;
	uint signature;
	ubyte* content;
	uint crc;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PngIENDChunk
{
	uint length;
	uint signature;
	uint crc;
};
#pragma pack(pop)

struct PngImage
{
	ubyte4* rgbaBitmap;
	int2 size;
};

PngImage LoadPngFile(const wchar_t* fileName);