#pragma once

#include "window_data.h"
#include "math.h"

int2 ConvertFromScreenToDrawingCoords(WindowData* windowData, int2 point);

void RecreateBackgroundBmp(WindowData* windowData);

void InitRenderer(WindowData* windowData, HWND hwnd);

void ValidateDrawingOffset(WindowData* windowData);
void CalculateDrawingZoneSize(WindowData* windowData);

void DrawLine(ubyte4* bitmap, int2 bitmapSize, int4 drawingRect, int2 from, int2 to, ubyte3 color);
void FillBitmapWithWhite(ubyte4* bitmap, int2 bitmapSize);

void FillFromPixel(WindowData* windowData, int2 fromPixel, ubyte3 color);

void CopyTextBufferToCanvas(WindowData* windowData);

void DrawRect(Bitmap bitmap, int2 bottomLeft, int2 size, ubyte3 color);
void DrawBorderRect(Bitmap bitmap, int2 bottomLeft, int2 size, int lineWidth, ubyte3 color);

int4 ClipRect(int4 rectSource, int4 rectDest);
int4 ClipRect(int4 rectSource, int2 rectDest);
int2 ClipPoint(int2 point, int4 rect);
int ClipPoint(int point, int2 range);

inline ubyte3 GetPixelColor(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord);
inline void DrawPixel(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord, ubyte3 color);

void DrawBitmap(WindowData* windowData, ubyte4* bitmapToCopy, int2 topLeft, int2 bitmapSize);
void CopyBitmapToBitmap(ubyte4* sourceBitmap, int2 sourceBitmapSize,
	ubyte4* destBitmap, int2 destXY, int2 destBitmapSize);

void CopyMonochromicBitmapToBitmap(ubyte* sourceBitmap, int2 sourceBitmapSize,
	ubyte4* destBitmap, int2 destXY, int2 destBitmapSize, int zoom = 1, bool invertColor = false);
