#pragma once

#include "window_data.h"

void RecreateBackgroundBmp(WindowData* windowData);

void InitRenderer(WindowData* windowData, HWND hwnd);

void ValidateDrawingOffset(WindowData* windowData);
void CalculateDrawingZone(WindowData* windowData);

void DrawLine(ubyte4* bitmap, int2 bitmapSize, int2 from, int2 to, ubyte3 color);
void FillBitmapWithWhite(ubyte4* bitmap, int2 bitmapSize);

void FillFromPixel(WindowData* windowData, int2 fromPixel, ubyte3 color);

void DrawRect(WindowData* windowData, int x, int y, int width, int height, ubyte3 color);
void DrawBorderRect(WindowData* windowData, int2 bottomLeft, int2 size, int lineWidth, ubyte3 color);

inline ubyte3 GetPixelColor(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord);
inline void DrawPixel(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord, ubyte3 color);

void DrawBitmap(WindowData* windowData, ubyte4* bitmapToCopy, int2 topLeft, int2 bitmapSize);
void CopyBitmapToBitmap(ubyte4* sourceBitmap, int2 sourceBitmapSize,
	ubyte4* destBitmap, int2 destXY, int2 destBitmapSize);
