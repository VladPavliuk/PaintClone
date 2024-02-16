#pragma once

//#include "custom_types.h"
#include "files.h"
#include "hash_table.h"
#include "math.h"
#include "common.h"
#include "renderer.h"

struct FontGlyph
{
	wchar_t code;
	int4 coordsRect;

	SimpleDynamicArray<SimpleDynamicArray<int2>> contours; // contours a list of closed polygons
};

struct RasterizedGlyph
{
	wchar_t code;
	int2 bitmapSize;
	ubyte4* bitmap;

	// TEMPORARY FOR TESTING
	//SimpleDynamicArray<SimpleDynamicArray<int2>> contours;
};

//> TTF structs
struct TTFTableRecord
{
	char tag[4];
	uint checksum;
	uint offset;
	uint length;
};

#pragma pack(push,2)
struct TTFFileHeader
{
	uint sfntVersion;
	ushort numTables;
	ushort searchRange;
	ushort entrySelector;
	ushort rangeShift;
	//TODO: clear it as well
	SimpleDynamicArray<TTFTableRecord> tableRecords;
};
#pragma pack(pop)

#pragma pack(push,2)
struct TTFheadTableHeader
{
	ushort majorVersion;
	ushort minorVersion;
	float fontRevision;
	uint checksumAdjustment;
	uint magicNumber;
	ushort flags;
	ushort unitsPerEm;
	long long created;
	long long modified;
	short xMin;
	short yMin;
	short xMax;
	short yMax;
	ushort macStyle;
	ushort lowestRecPPEM;
	short fontDirectionHint;
	short indexToLocFormat;
	short glyphDataFormat;
};
#pragma pack(pop)

struct TTFlocaTableShort
{
	SimpleDynamicArray<ushort> offsets;
};

struct TTFlocaTableLong
{
	SimpleDynamicArray<uint> offsets;
};

// for version 0.5
struct TTFmaxpTableVersion1
{
	uint version; // should be 0x00005000
	ushort numGlyphs;
};

// for version 1.0
struct TTFmaxpTableVersion2
{
	uint version; // should be 0x00010000
	ushort numGlyphs;
	ushort maxPoints;
	ushort maxContours;
	ushort maxCompositePoints;
	ushort maxCompositeContours;
	ushort maxZones;
	ushort maxTwilightPoints;
	ushort maxStorage;
	ushort maxFunctionDefs;
	ushort maxInstructionDefs;
	ushort maxStackElements;
	ushort maxSizeOfInstructions;
	ushort maxComponentElements;
	ushort maxComponentDepth;
};

struct TTFcmapEncodingRecord
{
	ushort platformID;
	ushort encodingID;
	uint subtableOffset;
};

#pragma pack(push,2)
struct TTFcmapTableHeader
{
	ushort version;
	ushort numTables;
	SimpleDynamicArray<TTFcmapEncodingRecord> encodingsRecords;
};
#pragma pack(pop)

struct TTFcmapSubtableFormat4
{
	ushort format;
	ushort length; // length of current subtable in bytes
	ushort language;
	ushort segCountX2;
	ushort searchRange;
	ushort entrySelector;
	ushort rangeShift;
	SimpleDynamicArray<ushort> endCodes; // array
	ushort reservedPad; // always 0
	SimpleDynamicArray<ushort> startCodes; // array
	SimpleDynamicArray<short> idDeltas; // array
	SimpleDynamicArray<ushort> idRangeOffsets; // array
	ushort glyphIdArray; // array
};

struct TTFglyfHeader
{
	short numberOfContours; // if greater or equal then 0, then it's a simple glyph
	short xMin;
	short yMin;
	short xMax;
	short yMax;
};

struct TTFSimpleGlyph
{
	SimpleDynamicArray<ushort> endPtsOfContours;
	ushort instructionLength;
	SimpleDynamicArray<ubyte> instructions;
	SimpleDynamicArray<ubyte> flags;

	// even though xCoordinate or yCoordinate can be 8 or 16 bit length
	// we use 16 bit, since it covers 8 bit as well
	// for 8 bit we will use lower 8 bits
	SimpleDynamicArray<short> xCoordinates;
	SimpleDynamicArray<short> yCoordinates;
};
//<

RasterizedGlyph RasterizeFontGlyph(FontGlyph fontGlyph, int glyphPixelsHeight);
HashTable<RasterizedGlyph> RasterizeFontGlyphs(HashTable<FontGlyph>* fontGlyphs);
HashTable<FontGlyph> ReadGlyphsFromTTF(const wchar_t* ttfFilePath, SimpleDynamicArray<wchar_t>* alphabet);