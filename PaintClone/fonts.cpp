#include "fonts.h"

void FillClosedPolygon(ubyte* bitmap, int2 bitmapSize,
	SimpleDynamicArray<SimpleDynamicArray<float2>>* contours)
{
	int height = bitmapSize.y;
	int width = bitmapSize.x;

	int totalPointsCount = 0;

	for (int i = 0; i < contours->length; i++)
	{
		totalPointsCount += contours->length;
	}

	// In order to use the algorithm, we should create a list of edges (line connection points)
	SimpleDynamicArray<float4> edges = SimpleDynamicArray<float4>(totalPointsCount);
	for (int i = 0; i < contours->length; i++)
	{
		SimpleDynamicArray<float2>* points = contours->getPointer(i);

		for (int j = 0; j < points->length - 1; j++)
		{
			float2 topPoint = points->get(j);
			float2 bottomPoint = points->get(j + 1);

			// edges should be directed from top to bottom
			if (bottomPoint.y > topPoint.y)
			{
				float2 tmp = bottomPoint;
				bottomPoint = topPoint;
				topPoint = tmp;
			}

			edges.add({ topPoint.x, topPoint.y, bottomPoint.x, bottomPoint.y });
		}
	}

	// BEGIN FILL ALGORITHM
	SimpleDynamicArray<float4> activeEdges = SimpleDynamicArray<float4>(10);
	SimpleDynamicArray<float> xIntersections = SimpleDynamicArray<float>(10);

	int verticalSubSteps = 4;
	float pixelWeightPerIteration = 255.0f / (float)verticalSubSteps;
	for (float scanline = 0; scanline <= height; scanline += 1.0f / verticalSubSteps)
	{
		activeEdges.clear();
		xIntersections.clear();

		// check for active edges
		for (int i = 0; i < edges.length; i++)
		{
			float4 edge = edges.get(i);
			float2 topPoint = { edge.x, edge.y };
			float2 bottomPoint = { edge.z, edge.w };

			// NOTE: the algorithm is VERY sensitive to this boundary check!
			if (scanline > bottomPoint.y && scanline <= topPoint.y)
			{
				activeEdges.add(edge);
			}
		}

		// find intersection points
		for (int i = 0; i < activeEdges.length; i++)
		{
			float4 activeEdge = activeEdges.get(i);

			float2 topPoint = { activeEdge.x, activeEdge.y };
			float2 bottomPoint = { activeEdge.z, activeEdge.w };

			if (topPoint.y - bottomPoint.y == 0) continue;

			float slope = (topPoint.y - bottomPoint.y) / (topPoint.x - bottomPoint.x);

			float xIntersection = bottomPoint.x + (float)(scanline - bottomPoint.y) / slope;
			xIntersections.add(xIntersection);
		}

		//if (xIntersections.length == 1) continue;

		// sort intersections from left to right
		BubbleSort(&xIntersections);

		// fill scanline
		for (int i = 0; i < xIntersections.length - 1; i += 2)
		{
			float xFromPoint = xIntersections.get(i);
			int xFromPixel = (int)xFromPoint;

			float xToPoint = xIntersections.get(i + 1);
			int xToPixel = (int)xToPoint;

			ubyte leftCornerColor = (ubyte)(pixelWeightPerIteration * ((float)(xFromPixel + 1) - xFromPoint));

			if (xFromPixel == xToPixel)
			{
				bitmap[xFromPixel + (int)scanline * width] += leftCornerColor;
			}
			else
			{
				bitmap[xFromPixel + (int)scanline * width] += leftCornerColor;

				for (int j = xFromPixel + 1; j < xToPixel; j++)
				{
					bitmap[j + (int)scanline * width] += (ubyte)pixelWeightPerIteration;
				}

				ubyte rightCornerColor = (ubyte)(pixelWeightPerIteration * ((float)(xToPoint - (float)xToPixel)));
				bitmap[xToPixel + (int)scanline * width] += rightCornerColor;
			}
		}
	}

	activeEdges.freeMemory();
	xIntersections.freeMemory();
	edges.freeMemory();
}

void RasterizeFontGlyph(FontGlyph fontGlyph, int glyphPixelsHeight, ubyte** bitmap, int2* bitmapSize)
{
	int fontGlyphHeight = fontGlyph.coordsRect.w - fontGlyph.coordsRect.y;
	int fontGlyphWidth = fontGlyph.coordsRect.z - fontGlyph.coordsRect.x;

	float scaleRatio = (float)glyphPixelsHeight / (float)fontGlyphHeight;

	fontGlyphWidth = (int)((float)fontGlyphWidth * scaleRatio);
	fontGlyphHeight = glyphPixelsHeight;

	*bitmapSize = { fontGlyphWidth + 2, fontGlyphHeight + 1 };

	// Scale points to the desired rasterized size
	// and remove duplicated points
	SimpleDynamicArray<SimpleDynamicArray<float2>> scaledContours = SimpleDynamicArray<SimpleDynamicArray<float2>>(fontGlyph.contours.length);
	//bool* duplicatedPointsMap = (bool*)malloc(rasterizedGlyph.bitmapSize.x * rasterizedGlyph.bitmapSize.y);
	//ZeroMemory(duplicatedPointsMap, rasterizedGlyph.bitmapSize.x * rasterizedGlyph.bitmapSize.y);

	for (int i = 0; i < fontGlyph.contours.length; i++)
	{
		SimpleDynamicArray<int2> contour = fontGlyph.contours.get(i);
		SimpleDynamicArray<float2> scaledContour = SimpleDynamicArray<float2>(contour.length);

		for (int j = 0; j < contour.length; j++)
		{
			int2 point = contour.get(j);
			float2 scaledPoint = { (float)point.x, (float)point.y };

			scaledPoint.x -= fontGlyph.coordsRect.x;
			scaledPoint.y -= fontGlyph.coordsRect.y;

			scaledPoint.x *= scaleRatio;
			scaledPoint.y *= scaleRatio;

			/*if (!duplicatedPointsMap[point.x + point.y * rasterizedGlyph.bitmapSize.x])
			{
				duplicatedPointsMap[point.x + point.y * rasterizedGlyph.bitmapSize.x] = true;
				scaledContour.add(point);
			}*/

			scaledContour.add(scaledPoint);
		}

		/*if (scaledContour.length > 0)
		{
			scaledContours.add(scaledContour);
		}*/
		scaledContours.add(scaledContour);
	}
	//free(duplicatedPointsMap);

	// Since we removed all duplicated points, we also removed first and last points, that should be the same (in order to have closed polygon)
	for (int i = 0; i < scaledContours.length; i++)
	{
		SimpleDynamicArray<float2>* contour = scaledContours.getPointer(i);

		contour->add(contour->get(0));
	}

	*bitmap = (ubyte*)malloc((*bitmapSize).x * (*bitmapSize).y);

	//FillBitmapWithWhite(rasterizedGlyph.bitmap, rasterizedGlyph.bitmapSize);

	// black background
	ZeroMemory(*bitmap, (*bitmapSize).x * (*bitmapSize).y);

	//> create outlline of glyph
	// Uncomment to create outline of points
	//for (int i = 0; i < scaledContours.length; i++)
	//{
	//	SimpleDynamicArray<float2> contour = scaledContours.get(i);
	//	SimpleDynamicArray<int2> scaledContour = SimpleDynamicArray<int2>(contour.length);

	//	for (int j = 0; j < contour.length - 1; j++)
	//	{
	//		float2 point = contour.get(j);
	//		float2 nextPoint = contour.get(j + 1);

	//		// plot a single point
	//		//rasterizedGlyph.bitmap[point.x + point.y * rasterizedGlyph.bitmapSize.x] = { 255, 0, 0, 0 };

	//		int4 boundaries = { 0, 0, rasterizedGlyph.bitmapSize.x, rasterizedGlyph.bitmapSize.y };
	//		DrawLine(rasterizedGlyph.bitmap, rasterizedGlyph.bitmapSize,
	//			boundaries, { (int)point.x, (int)point.y }, { (int)nextPoint.x, (int)nextPoint.y }, { 123, 0, 0 });
	//	}
	//}
	//<

	FillClosedPolygon(*bitmap, *bitmapSize, &scaledContours);

	for (int i = 0; i < scaledContours.length; i++)
	{
		scaledContours.get(i).freeMemory();
	}
	scaledContours.freeMemory();
}

FontDataRasterized RasterizeFontGlyphs(FontData* font, int maxSymbolPixelsHeight)
{
	FontDataRasterized fontDataRasterized;
	fontDataRasterized.glyphs = HashTable<RasterizedGlyph>(font->glyphs._capacity);

	int maxWidth = font->maxBoundaries.z - font->maxBoundaries.x;
	int maxHeight = font->maxBoundaries.w - font->maxBoundaries.y;

	float scaleRatio = (float)maxSymbolPixelsHeight / (float)maxHeight;

	fontDataRasterized.lineHeight = (int)((float)font->lineHeight * scaleRatio);
	fontDataRasterized.ascent = (int)((float)font->ascent * scaleRatio);
	fontDataRasterized.descent = (int)((float)font->descent * scaleRatio);
	fontDataRasterized.lineGap = (int)((float)font->lineGap * scaleRatio);

	font->glyphs.resetIteration();
	HashTableItem<int, FontGlyph> fontGlyphItem;
	while (font->glyphs.getNext(&fontGlyphItem))
	{
		FontGlyph fontGlyph = fontGlyphItem.value;
		int glyphHeight = fontGlyph.coordsRect.w - fontGlyph.coordsRect.y;
		int glyphScaledHeight = (int)((float)glyphHeight * scaleRatio);

		RasterizedGlyph rasterizedGlyph;
		rasterizedGlyph.code = fontGlyph.code;
		rasterizedGlyph.leftSideBearings = (int)(scaleRatio * (float)fontGlyph.leftSideBearings);
		rasterizedGlyph.advanceWidth = (int)(scaleRatio * (float)fontGlyph.advanceWidth);
		rasterizedGlyph.hasBitmap = fontGlyph.hasContours;

		if (!rasterizedGlyph.hasBitmap)
		{
			rasterizedGlyph.boundaries = { 0, 0, 0, 0 };
			rasterizedGlyph.bitmap = 0;
			rasterizedGlyph.bitmapSize = { -1, -1 };
		}
		else
		{
			RasterizeFontGlyph(fontGlyph, glyphScaledHeight, &rasterizedGlyph.bitmap, &rasterizedGlyph.bitmapSize);
			rasterizedGlyph.boundaries = fontGlyph.coordsRect * scaleRatio;
		}

		fontDataRasterized.glyphs.set(rasterizedGlyph.code, rasterizedGlyph);
	}

	return fontDataRasterized;
}

// PERFORMANCE +-3 ml sec
FontData ReadFontFromTTF(const wchar_t* ttfFilePath, SimpleDynamicArray<wchar_t>* alphabet)
{
	FontData fontData;

	ubyte* ttfFileBuffer;
	int ttfFileSize;
	ReadFileIntoBuffer(ttfFilePath, &ttfFileBuffer, &ttfFileSize);

	ubyte* ttfFileBufferCursor = ttfFileBuffer;

	TTFFileHeader ttfFileHeader = *((TTFFileHeader*)ttfFileBufferCursor);

	// TTF file is always Big Endian, so we swap it
	ttfFileHeader.numTables = _byteswap_ushort(ttfFileHeader.numTables);

	ttfFileBufferCursor += sizeof(ttfFileHeader);

	ttfFileBufferCursor -= sizeof(ttfFileHeader.tableRecords);

	// TODO: Remove table records array if not needed
	ttfFileHeader.tableRecords = SimpleDynamicArray<TTFTableRecord>(ttfFileHeader.numTables);

	TTFTableRecord cmapTableRecord = {};
	TTFTableRecord glyfTableRecord = {};
	TTFTableRecord maxpTableRecord = {};
	TTFTableRecord locaTableRecord = {};
	TTFTableRecord headTableRecord = {};
	TTFTableRecord hmtxTableRecord = {};
	TTFTableRecord hheaTableRecord = {};
	TTFTableRecord cvtTableRecord = {};
	TTFTableRecord fpgmTableRecord = {};
	TTFTableRecord prepTableRecord = {};

	for (int i = 0; i < ttfFileHeader.numTables; i++)
	{
		TTFTableRecord ttfTableRecord = *((TTFTableRecord*)ttfFileBufferCursor);

		ttfTableRecord.offset = _byteswap_ulong(ttfTableRecord.offset);
		ttfTableRecord.length = _byteswap_ulong(ttfTableRecord.length);

		if (CompareStrings("cmap", 4, (const char*)ttfTableRecord.tag, 4))
		{
			cmapTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("glyf", 4, (const char*)ttfTableRecord.tag, 4))
		{
			glyfTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("maxp", 4, (const char*)ttfTableRecord.tag, 4))
		{
			maxpTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("head", 4, (const char*)ttfTableRecord.tag, 4))
		{
			headTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("loca", 4, (const char*)ttfTableRecord.tag, 4))
		{
			locaTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("hmtx", 4, (const char*)ttfTableRecord.tag, 4))
		{
			hmtxTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("hhea", 4, (const char*)ttfTableRecord.tag, 4))
		{
			hheaTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("fpgm", 4, (const char*)ttfTableRecord.tag, 4))
		{
			fpgmTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("cvt ", 4, (const char*)ttfTableRecord.tag, 4))
		{
			cvtTableRecord = ttfTableRecord;
		}
		else if (CompareStrings("prep", 4, (const char*)ttfTableRecord.tag, 4))
		{
			prepTableRecord = ttfTableRecord;
		}

		ttfFileBufferCursor += sizeof(TTFTableRecord);
	}

	//> head table
	ttfFileBufferCursor = ttfFileBuffer + headTableRecord.offset;

	TTFheadTableHeader ttfHeadTableHeader = *(TTFheadTableHeader*)ttfFileBufferCursor;
	ttfHeadTableHeader.magicNumber = _byteswap_ulong(ttfHeadTableHeader.magicNumber);
	ttfHeadTableHeader.indexToLocFormat = _byteswap_ushort(ttfHeadTableHeader.indexToLocFormat);
	ttfHeadTableHeader.xMin = _byteswap_ushort(ttfHeadTableHeader.xMin);
	ttfHeadTableHeader.yMin = _byteswap_ushort(ttfHeadTableHeader.yMin);
	ttfHeadTableHeader.xMax = _byteswap_ushort(ttfHeadTableHeader.xMax);
	ttfHeadTableHeader.yMax = _byteswap_ushort(ttfHeadTableHeader.yMax);

	fontData.maxBoundaries = { ttfHeadTableHeader.xMin ,ttfHeadTableHeader.yMin, ttfHeadTableHeader.xMax, ttfHeadTableHeader.yMax };
	if (ttfHeadTableHeader.magicNumber != 0x5F0F3CF5)
	{
		assert(false);
	}
	//<


	// maxp table
	int numberOfGlyphs = -1;

	ttfFileBufferCursor = ttfFileBuffer + maxpTableRecord.offset;
	TTFmaxpTableVersion2 ttfMaxpTableVersion2 = *(TTFmaxpTableVersion2*)ttfFileBufferCursor;

	ttfMaxpTableVersion2.version = _byteswap_ulong(ttfMaxpTableVersion2.version);
	if (ttfMaxpTableVersion2.version == 0x00005000)
	{
		assert(false);
	}
	ttfMaxpTableVersion2.numGlyphs = _byteswap_ushort(ttfMaxpTableVersion2.numGlyphs);
	ttfMaxpTableVersion2.maxPoints = _byteswap_ushort(ttfMaxpTableVersion2.maxPoints);
	ttfMaxpTableVersion2.maxContours = _byteswap_ushort(ttfMaxpTableVersion2.maxContours);
	ttfMaxpTableVersion2.maxCompositePoints = _byteswap_ushort(ttfMaxpTableVersion2.maxCompositePoints);
	ttfMaxpTableVersion2.maxCompositeContours = _byteswap_ushort(ttfMaxpTableVersion2.maxCompositeContours);
	ttfMaxpTableVersion2.maxZones = _byteswap_ushort(ttfMaxpTableVersion2.maxZones);
	ttfMaxpTableVersion2.maxTwilightPoints = _byteswap_ushort(ttfMaxpTableVersion2.maxTwilightPoints);
	ttfMaxpTableVersion2.maxStorage = _byteswap_ushort(ttfMaxpTableVersion2.maxStorage);
	ttfMaxpTableVersion2.maxFunctionDefs = _byteswap_ushort(ttfMaxpTableVersion2.maxFunctionDefs);
	ttfMaxpTableVersion2.maxInstructionDefs = _byteswap_ushort(ttfMaxpTableVersion2.maxInstructionDefs);
	ttfMaxpTableVersion2.maxStackElements = _byteswap_ushort(ttfMaxpTableVersion2.maxStackElements);
	ttfMaxpTableVersion2.maxSizeOfInstructions = _byteswap_ushort(ttfMaxpTableVersion2.maxSizeOfInstructions);
	ttfMaxpTableVersion2.maxComponentElements = _byteswap_ushort(ttfMaxpTableVersion2.maxComponentElements);
	ttfMaxpTableVersion2.maxComponentDepth = _byteswap_ushort(ttfMaxpTableVersion2.maxComponentDepth);

	numberOfGlyphs = ttfMaxpTableVersion2.numGlyphs;

	//> hhea table
	ttfFileBufferCursor = ttfFileBuffer + hheaTableRecord.offset;

	TTFhheaTable hheaTable = *(TTFhheaTable*)ttfFileBufferCursor;
	hheaTable.ascender = _byteswap_ushort(hheaTable.ascender);
	hheaTable.descender = _byteswap_ushort(hheaTable.descender);
	hheaTable.lineGap = _byteswap_ushort(hheaTable.lineGap);
	hheaTable.numberOfHMetrics = _byteswap_ushort(hheaTable.numberOfHMetrics);

	fontData.ascent = hheaTable.ascender;
	fontData.descent = hheaTable.descender;
	fontData.lineGap = hheaTable.lineGap;
	fontData.lineHeight = hheaTable.ascender + -hheaTable.descender + hheaTable.lineGap;
	//<

	//> hmtx table
	TTFhmtxTable ttfHmtxTable;
	ttfFileBufferCursor = ttfFileBuffer + hmtxTableRecord.offset;

	ttfHmtxTable.hMetrics = SimpleDynamicArray<TTFlongHorMetricRecord>(hheaTable.numberOfHMetrics);

	for (int i = 0; i < hheaTable.numberOfHMetrics; i++)
	{
		TTFlongHorMetricRecord record = *(TTFlongHorMetricRecord*)ttfFileBufferCursor;

		record.advanceWidth = _byteswap_ushort(record.advanceWidth);
		record.lsb = _byteswap_ushort(record.lsb);
		ttfHmtxTable.hMetrics.add(record);

		ttfFileBufferCursor += sizeof(TTFlongHorMetricRecord);
	}
	//<

	//> loca table
	TTFlocaTableLong ttfLocaTableLong;

	ttfLocaTableLong.offsets = SimpleDynamicArray<uint>(numberOfGlyphs + 1);

	ttfFileBufferCursor = ttfFileBuffer + locaTableRecord.offset;

	if (ttfHeadTableHeader.indexToLocFormat == 1)
	{
		// long
		for (int i = 0; i < numberOfGlyphs + 1; i++)
		{
			uint offset = _byteswap_ulong(*(uint*)ttfFileBufferCursor);
			ttfLocaTableLong.offsets.add(offset);

			ttfFileBufferCursor += sizeof(uint);
		}
	}
	else if (ttfHeadTableHeader.indexToLocFormat == 0)
	{
		// short
		for (int i = 0; i < numberOfGlyphs + 1; i++)
		{
			uint offset = (uint)_byteswap_ushort(*(ushort*)ttfFileBufferCursor);
			ttfLocaTableLong.offsets.add(offset * 2); // NOTE: for some reasons offset divided by 2 stored

			ttfFileBufferCursor += sizeof(ushort);
		}
	}
	else
	{
		// wrong file
		assert(false);
	}
	//<

	ushort* charToGlyphIdMapping = (ushort*)malloc(USHRT_MAX * sizeof(ushort));
	ZeroMemory(charToGlyphIdMapping, USHRT_MAX * sizeof(ushort));
	// cmap table
	{
		ttfFileBufferCursor = ttfFileBuffer + cmapTableRecord.offset;

		TTFcmapTableHeader ttfCmapTableHeader = *(TTFcmapTableHeader*)ttfFileBufferCursor;
		ttfFileBufferCursor += sizeof(ttfCmapTableHeader);

		// TODO: Remove table records array if not needed
		ttfFileBufferCursor -= sizeof(ttfCmapTableHeader.encodingsRecords);

		ttfCmapTableHeader.numTables = _byteswap_ushort(ttfCmapTableHeader.numTables);

		TTFcmapEncodingRecord ttfWindowCmapEncodingRecord = {};
		for (int i = 0; i < ttfCmapTableHeader.numTables; i++)
		{
			TTFcmapEncodingRecord ttfCmapEncodingRecord = *((TTFcmapEncodingRecord*)ttfFileBufferCursor);

			ttfCmapEncodingRecord.platformID = _byteswap_ushort(ttfCmapEncodingRecord.platformID);
			ttfCmapEncodingRecord.encodingID = _byteswap_ushort(ttfCmapEncodingRecord.encodingID);
			ttfCmapEncodingRecord.subtableOffset = _byteswap_ulong(ttfCmapEncodingRecord.subtableOffset);

			// NOTE: for now we are interested only in windows platform
			if (ttfCmapEncodingRecord.platformID == 3)
			{
				ttfWindowCmapEncodingRecord = ttfCmapEncodingRecord;
			}

			ttfFileBufferCursor += sizeof(TTFcmapEncodingRecord);
		}

		ttfFileBufferCursor = ttfFileBuffer + cmapTableRecord.offset + ttfWindowCmapEncodingRecord.subtableOffset;

		ushort subtableFormat = _byteswap_ushort(*(ushort*)ttfFileBufferCursor);

		if (subtableFormat == 4)
		{
			TTFcmapSubtableFormat4 format4 = *(TTFcmapSubtableFormat4*)ttfFileBufferCursor;

			format4.segCountX2 = _byteswap_ushort(format4.segCountX2);

			int segmentsCount = format4.segCountX2 / 2;

			ttfFileBufferCursor = ttfFileBufferCursor + 14; // skiping all fields utill endCode point

			format4.endCodes = SimpleDynamicArray<ushort>(segmentsCount);
			for (int i = 0; i < segmentsCount; i++)
			{
				format4.endCodes.add(_byteswap_ushort(*(ushort*)ttfFileBufferCursor));

				ttfFileBufferCursor += sizeof(ushort);
			}

			ttfFileBufferCursor += sizeof(ushort); // skip reservedPad

			format4.startCodes = SimpleDynamicArray<ushort>(segmentsCount);
			for (int i = 0; i < segmentsCount; i++)
			{
				format4.startCodes.add(_byteswap_ushort(*(ushort*)ttfFileBufferCursor));

				ttfFileBufferCursor += sizeof(ushort);
			}

			format4.idDeltas = SimpleDynamicArray<short>(segmentsCount);
			for (int i = 0; i < segmentsCount; i++)
			{
				format4.idDeltas.add(_byteswap_ushort(*(short*)ttfFileBufferCursor));

				ttfFileBufferCursor += sizeof(short);
			}

			format4.idRangeOffsets = SimpleDynamicArray<ushort>(segmentsCount);
			ubyte* idRangeOffsetsStart = ttfFileBufferCursor;

			for (int i = 0; i < segmentsCount; i++)
			{
				format4.idRangeOffsets.add(_byteswap_ushort(*(ushort*)ttfFileBufferCursor));

				ttfFileBufferCursor += sizeof(ushort);
			}

			for (int i = 0; i < segmentsCount; i++)
			{
				int endCode = format4.endCodes.get(i);
				int startCode = format4.startCodes.get(i);
				short idDelta = format4.idDeltas.get(i);
				ushort idRangeOffset = format4.idRangeOffsets.get(i);

				for (int c = startCode; c <= endCode; c++)
				{
					ushort glyphId = 0; // index in glyf table

					if (idRangeOffset == 0)
					{
						glyphId = (c + idDelta) & 0xFFFF;
					}
					else
					{
						int startCodeOffset = (c - startCode) * 2;
						int currentRangeOffset = i * 2;

						ubyte* glyphIndexOffset =
							idRangeOffsetsStart + // where all offsets started
							currentRangeOffset + // offset for the current range
							idRangeOffset + // offset between the id range table and the glyphIdArray[]
							startCodeOffset; // gets us finally to the character

						glyphId = _byteswap_ushort(*(ushort*)glyphIndexOffset);
						if (glyphId != 0)
						{
							glyphId = (glyphId + idDelta) & 0xFFFF;
						}
					}

					// if glyphId, that means utf char has no glyph in the font 
					if (glyphId != 0)
					{
						charToGlyphIdMapping[c] = glyphId;
					}
				}
			}

			format4.endCodes.freeMemory();
			format4.startCodes.freeMemory();
			format4.idDeltas.freeMemory();
			format4.idRangeOffsets.freeMemory();
		}
		else
		{
			// format is not implemented!
			assert(false);
		}
	}

	//> cvt table
	TTFcvtTable cvtTable;
	int cvtValuesCount = cvtTableRecord.length / sizeof(short);
	cvtTable.values = SimpleDynamicArray<short>(cvtValuesCount);
	cvtTable.values.length = cvtValuesCount;

	ttfFileBufferCursor = ttfFileBuffer + cvtTableRecord.offset;

	memcpy(cvtTable.values._elements, ttfFileBufferCursor, cvtTableRecord.length);
	//<

	//> fpgm table
	TTFfpgmTable fpgmTable;

	fpgmTable.instructions = SimpleDynamicArray<ubyte>(fpgmTableRecord.length);
	fpgmTable.instructions.length = fpgmTableRecord.length;

	ttfFileBufferCursor = ttfFileBuffer + fpgmTableRecord.offset;

	memcpy(fpgmTable.instructions._elements, ttfFileBufferCursor, fpgmTableRecord.length);
	//<

	//> prep table
	TTFprepTable prepTable;

	prepTable.instructions = SimpleDynamicArray<ubyte>(prepTableRecord.length);
	prepTable.instructions.length = prepTableRecord.length;

	ttfFileBufferCursor = ttfFileBuffer + prepTableRecord.offset;

	memcpy(prepTable.instructions._elements, ttfFileBufferCursor, prepTableRecord.length);
	//<

	TTFProgram program;

	program.functions = SimpleDynamicArray<TTFProgramFunction>::allocate(ttfMaxpTableVersion2.maxFunctionDefs);
	program.stack = Stack<uint>::allocate(ttfMaxpTableVersion2.maxStackElements);

	//> Execute fpgm instructions
	program.fpgmInstructions.instructions = (ubyte*)malloc(fpgmTable.instructions.length);
	memcpy(program.fpgmInstructions.instructions, fpgmTable.instructions._elements, fpgmTable.instructions.length);
	program.fpgmInstructions.length = fpgmTable.instructions.length;
	
	program.instructions = program.fpgmInstructions.instructions;
	program.instructionsLength = program.fpgmInstructions.length;
	program.currentInstruction = program.instructions;

	ConsolePrint("FPGM program\n");
	TTFExecuteProgram(program);

	program.stack->freeMemory();
	//<

	//> Prepare CV values
	program.cv = SimpleDynamicArray<int>::allocate(cvtTable.values.length);
	for (int i = 0; i < cvtTable.values.length; i++)
	{
		program.cv->add(cvtTable.values.get(i));
	}
	//<
	program.stack = Stack<uint>::allocate(ttfMaxpTableVersion2.maxStackElements);

	//> Execute prep instructions
	program.prepInstructions.instructions = (ubyte*)malloc(prepTable.instructions.length);
	memcpy(program.prepInstructions.instructions, prepTable.instructions._elements, prepTable.instructions.length);
	program.prepInstructions.length = prepTable.instructions.length;

	program.instructions = program.prepInstructions.instructions;
	program.instructionsLength = program.prepInstructions.length;
	program.currentInstruction = program.instructions;

	ConsolePrint("PREP program\n");
	TTFExecuteProgram(program);

	program.stack->freeMemory();
	//<

	// glyf table
	fontData.glyphs = HashTable<FontGlyph>(alphabet->length);
	// GLYPHS DATA READING
	for (int codeIndex = 0; codeIndex < alphabet->length; codeIndex++)
	{
		wchar_t code = alphabet->get(codeIndex);

		FontGlyph glyph;
		glyph.code = code;

		int glyphId = charToGlyphIdMapping[code];

		// no glyph for the symbol
		if (glyphId == 0)
		{
			assert(false);
		}

		TTFlongHorMetricRecord longHorMetricRecord = ttfHmtxTable.hMetrics.get(glyphId);

		glyph.leftSideBearings = longHorMetricRecord.lsb;
		glyph.advanceWidth = longHorMetricRecord.advanceWidth;

		if (code == L' ')
		{
			glyph.coordsRect = { 0, 0, 0, 0 };
			glyph.hasContours = false;
			fontData.glyphs.set(glyph.code, glyph);
			continue;
		}

		glyph.hasContours = true;

		// NOTE: usually we will have 2 contours (one for inner outline, second for outer outline)
		glyph.contours = SimpleDynamicArray<SimpleDynamicArray<int2>>(2);

		int offsetToGlyph = ttfLocaTableLong.offsets.get(glyphId);

		ttfFileBufferCursor = ttfFileBuffer + glyfTableRecord.offset + offsetToGlyph;

		// trying to find a glyph data from offset
		TTFglyfHeader ttfGlyfHeader = *(TTFglyfHeader*)ttfFileBufferCursor;
		ttfFileBufferCursor += sizeof(ttfGlyfHeader);

		ttfGlyfHeader.numberOfContours = _byteswap_ushort(ttfGlyfHeader.numberOfContours);
		ttfGlyfHeader.xMin = _byteswap_ushort(ttfGlyfHeader.xMin);
		ttfGlyfHeader.yMin = _byteswap_ushort(ttfGlyfHeader.yMin);
		ttfGlyfHeader.xMax = _byteswap_ushort(ttfGlyfHeader.xMax);
		ttfGlyfHeader.yMax = _byteswap_ushort(ttfGlyfHeader.yMax);

		glyph.coordsRect = { ttfGlyfHeader.xMin, ttfGlyfHeader.yMin, ttfGlyfHeader.xMax, ttfGlyfHeader.yMax };

		if (ttfGlyfHeader.numberOfContours < 0)
		{
			assert(false);
		}
		TTFSimpleGlyph ttfSimpleGlyph;
		ttfSimpleGlyph.endPtsOfContours = SimpleDynamicArray<ushort>(ttfGlyfHeader.numberOfContours);

		for (int i = 0; i < ttfGlyfHeader.numberOfContours; i++)
		{
			ttfSimpleGlyph.endPtsOfContours.add(_byteswap_ushort(*(ushort*)ttfFileBufferCursor));

			ttfFileBufferCursor += sizeof(ushort);
		}

		ttfSimpleGlyph.instructionLength = _byteswap_ushort(*(ushort*)ttfFileBufferCursor);

		ttfFileBufferCursor += sizeof(ttfSimpleGlyph.instructionLength);

		// for now just skip instructions
		ttfFileBufferCursor += sizeof(ubyte) * ttfSimpleGlyph.instructionLength;

		int lastIndex = ttfSimpleGlyph.endPtsOfContours.get(ttfSimpleGlyph.endPtsOfContours.length - 1);
		ttfSimpleGlyph.flags = SimpleDynamicArray<ubyte>(lastIndex + 1);

		for (int i = 0; i < (lastIndex + 1); i++)
		{
			ubyte flag = *(ubyte*)ttfFileBufferCursor;
			ttfSimpleGlyph.flags.add(flag);
			ttfFileBufferCursor++;

			bool isRepeat = flag & 0x08; // 0x08 - REPEAT_FLAG

			if (isRepeat)
			{
				ubyte repeatTimes = *(ubyte*)ttfFileBufferCursor;

				while (repeatTimes > 0)
				{
					repeatTimes--;
					i++;
					ttfSimpleGlyph.flags.add(flag);
				}
				ttfFileBufferCursor++;
			}
		}

		ttfSimpleGlyph.xCoordinates = SimpleDynamicArray<short>(lastIndex + 1);
		short previousCoord = 0;
		short currentCoord = 0;

		for (int i = 0; i < (lastIndex + 1); i++)
		{
			ubyte flag = ttfSimpleGlyph.flags.get(i);

			//flag = 0b00010010;
			//flag = 0b00000010;
			//flag = 0b00010000;

			int flagCombined = (flag & 0x02) >> 1 | (flag & 0x10) >> 3;

			switch (flagCombined)
			{
			case 0b00: // X_SHORT_VECTOR is 0 and X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 0 - just read 16 bit
				currentCoord = *(short*)ttfFileBufferCursor;
				ttfFileBufferCursor += sizeof(short);
				currentCoord = _byteswap_ushort(currentCoord);
				break;
			case 0b10: // X_SHORT_VECTOR is 0 and X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 1 - use previous coord
				currentCoord = 0;
				break;
			case 0b11: // X_SHORT_VECTOR is 1 and X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 1 - read 8 bits
				currentCoord = (short)*ttfFileBufferCursor;
				ttfFileBufferCursor++;
				break;
			case 0b01: // X_SHORT_VECTOR is 1 and X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 0 - read 8 bits and multiply by -1
				currentCoord = -1 * (short)*ttfFileBufferCursor;
				ttfFileBufferCursor++;
				break;
			}

			ttfSimpleGlyph.xCoordinates.add(currentCoord + previousCoord);
			previousCoord = currentCoord + previousCoord;
		}

		ttfSimpleGlyph.yCoordinates = SimpleDynamicArray<short>(lastIndex + 1);
		previousCoord = 0;
		currentCoord = 0;

		for (int i = 0; i < (lastIndex + 1); i++)
		{
			ubyte flag = ttfSimpleGlyph.flags.get(i);

			//flag = 0b00100100;
			//flag = 0b00000100;
			//flag = 0b00100000;

			int flagCombined = (flag & 0x04) >> 2 | (flag & 0x20) >> 4;

			switch (flagCombined)
			{
			case 0b00: // Y_SHORT_VECTOR is 0 and Y_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 0 - just read 16 bit
				currentCoord = *(short*)ttfFileBufferCursor;
				ttfFileBufferCursor += sizeof(short);
				currentCoord = _byteswap_ushort(currentCoord);
				break;
			case 0b10: // Y_SHORT_VECTOR is 0 and Y_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 1 - use previous coord
				currentCoord = 0;
				break;
			case 0b11: // Y_SHORT_VECTOR is 1 and Y_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 1 - read 8 bits
				currentCoord = (short)*ttfFileBufferCursor;
				ttfFileBufferCursor++;
				break;
			case 0b01: // Y_SHORT_VECTOR is 1 and Y_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR is 0 - read 8 bits and multiply by -1
				currentCoord = -1 * (short)*ttfFileBufferCursor;
				ttfFileBufferCursor++;
				break;
			}

			ttfSimpleGlyph.yCoordinates.add(currentCoord + previousCoord);
			previousCoord = currentCoord + previousCoord;
		}

		int startIndexInContur = 0;

		for (int i = 0; i < ttfGlyfHeader.numberOfContours; i++)
		{
			SimpleDynamicArray<int2> contourPoints = SimpleDynamicArray<int2>(50);

			int lastIndexInContour = ttfSimpleGlyph.endPtsOfContours.get(i);
			int contourLength = lastIndexInContour - startIndexInContur;
			int pointsGenerated = 0;

			// find first point that's on curve
			int firstPointsOnCurveIndex = -1;
			for (int j = 0; j < contourLength; j++)
			{
				int pointIndex = startIndexInContur + j;

				if (ttfSimpleGlyph.flags.get(pointIndex) & 0x01)
				{
					firstPointsOnCurveIndex = j;
					break;
				}
			}

			if (firstPointsOnCurveIndex == -1)
			{
				assert(false);
			}

			for (int j = firstPointsOnCurveIndex, pointsIterated = 0; pointsIterated <= contourLength; j++, pointsIterated++)
			{
				j = j % (contourLength + 1);
				int pointIndex = startIndexInContur + j;
				ubyte flag = ttfSimpleGlyph.flags.get(pointIndex);
				int x = (int)ttfSimpleGlyph.xCoordinates.get(pointIndex);
				int y = (int)ttfSimpleGlyph.yCoordinates.get(pointIndex);

				if (flag & 0x01) // ON_CURVE_POINT	
				{
					pointsGenerated++;
					contourPoints.add({ x, y });
					continue;
				}

				int2 p0 = contourPoints.get(contourPoints.length - 1);
				int2 p1 = { x, y };
				int nextPointIndex = startIndexInContur + ((j + 1) % (contourLength + 1));
				int2 p2 = {
					(int)ttfSimpleGlyph.xCoordinates.get(nextPointIndex),
					(int)ttfSimpleGlyph.yCoordinates.get(nextPointIndex)
				};

				ubyte p2Flag = ttfSimpleGlyph.flags.get(nextPointIndex);

				if (!(p2Flag & 0x01)) // ON_CURVE_POINT
				{
					p2.x = p1.x + (int)((float)(p2.x - p1.x) / 2.0f);
					p2.y = p1.y + (int)((float)(p2.y - p1.y) / 2.0f);
				}
				pointsGenerated += GenerateBezierCurve(p0, p1, p2, 5, &contourPoints);
			}

			int2 firstPointInContor = contourPoints.get(contourPoints.length - pointsGenerated);

			contourPoints.add(firstPointInContor);

			glyph.contours.add(contourPoints);
			startIndexInContur = lastIndexInContour + 1;
		}

		//char buff[100];
		//for (int i = 0; i < index; i++)
		//{
		//	int2 coord = glyph.points.get(i);

		//	sprintf_s(buff, "(%f, %f)\n", coord.x / 100.0f, coord.y / 100.0f);
		//	OutputDebugStringA(buff);
		//}

		ttfSimpleGlyph.endPtsOfContours.freeMemory();
		ttfSimpleGlyph.flags.freeMemory();
		ttfSimpleGlyph.xCoordinates.freeMemory();
		ttfSimpleGlyph.yCoordinates.freeMemory();

		fontData.glyphs.set(glyph.code, glyph);
	}

	ttfHmtxTable.hMetrics.freeMemory();
	free(charToGlyphIdMapping);
	ttfLocaTableLong.offsets.freeMemory();
	ttfFileHeader.tableRecords.freeMemory();
	free(ttfFileBuffer);

	return fontData;
}
