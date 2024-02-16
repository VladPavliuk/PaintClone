#include "renderer.h"

int2 ConvertFromScreenToDrawingCoords(WindowData* windowData, int2 point)
{
	point.x -= windowData->drawingZone.x;
	point.y -= windowData->drawingZone.y;

	point.x += windowData->drawingOffset.x;
	point.y += windowData->drawingOffset.y;

	point.x = (int)((float)point.x / (float)windowData->drawingZoomLevel);
	point.y = (int)((float)point.y / (float)windowData->drawingZoomLevel);

	return point;
}

void RecreateBackgroundBmp(WindowData* windowData)
{
	if (windowData->backgroundBmp)
	{
		DeleteObject(windowData->backgroundBmp);
	}

	windowData->backgroundBmp = CreateDIBSection(0, &windowData->windowBitmapInfo,
		DIB_RGB_COLORS, (void**)&windowData->windowBitmap, NULL, 0x0);

	SelectObject(windowData->backgroundDC, windowData->backgroundBmp);

	//TODO: Find a faster way to fill default color
	FillBitmapWithWhite(windowData->windowBitmap, windowData->windowClientSize);
}

void InitRenderer(WindowData* windowData, HWND hwnd)
{
	windowData->windowDC = GetDC(hwnd);
	windowData->backgroundDC = CreateCompatibleDC(windowData->windowDC);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	windowData->windowClientSize.x = clientRect.right - clientRect.left;
	windowData->windowClientSize.y = clientRect.bottom - clientRect.top;

	//> automatically select drawing bitmap based on initial window size
	//windowData->drawingBitmapSize.x = windowData->windowClientSize.x - windowData->drawingZonePosition.x;
	//windowData->drawingBitmapSize.y = windowData->windowClientSize.y - windowData->drawingZonePosition.y;
	//<

	windowData->windowBitmapInfo.bmiHeader.biSize = sizeof(windowData->windowBitmapInfo.bmiHeader);
	windowData->windowBitmapInfo.bmiHeader.biWidth = windowData->windowClientSize.x;
	windowData->windowBitmapInfo.bmiHeader.biHeight = windowData->windowClientSize.y;
	windowData->windowBitmapInfo.bmiHeader.biPlanes = 1;
	windowData->windowBitmapInfo.bmiHeader.biBitCount = 32;
	windowData->windowBitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->windowBitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->windowBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->windowBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->windowBitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->windowBitmapInfo.bmiHeader.biClrImportant = 0;
	
	windowData->drawingBitmapInfo.bmiHeader.biSize = sizeof(windowData->drawingBitmapInfo.bmiHeader);
	windowData->drawingBitmapInfo.bmiHeader.biWidth = windowData->drawingBitmapSize.x;
	windowData->drawingBitmapInfo.bmiHeader.biHeight = windowData->drawingBitmapSize.y;
	windowData->drawingBitmapInfo.bmiHeader.biPlanes = 1;
	windowData->drawingBitmapInfo.bmiHeader.biBitCount = 32;
	windowData->drawingBitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->drawingBitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->drawingBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->drawingBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->drawingBitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->drawingBitmapInfo.bmiHeader.biClrImportant = 0;

	windowData->drawingBitmap = (ubyte4*)malloc(4 * windowData->drawingBitmapSize.x * windowData->drawingBitmapSize.y);
	FillBitmapWithWhite(windowData->drawingBitmap, windowData->drawingBitmapSize);

	RecreateBackgroundBmp(windowData);
	CalculateDrawingZoneSize(windowData);

	SelectObject(windowData->windowDC, windowData->backgroundBmp);
}

void ValidateDrawingOffset(WindowData* windowData)
{
	if (windowData->drawingOffset.y < 0)
	{
		windowData->drawingOffset.y = 0;
	}
	else
	{
		int drawingZoneOffsetY = windowData->drawingZoomLevel * windowData->drawingBitmapSize.y
			- windowData->drawingZone.size.y - windowData->drawingOffset.y;

		if (drawingZoneOffsetY < 0)
		{
			windowData->drawingOffset.y += drawingZoneOffsetY;
		}
	}

	if (windowData->drawingOffset.x < 0)
	{
		windowData->drawingOffset.x = 0;
	}
	else
	{
		int drawingZoneOffsetX = windowData->drawingZoomLevel * windowData->drawingBitmapSize.x
			- windowData->drawingZone.size.x - windowData->drawingOffset.x;

		if (drawingZoneOffsetX < 0)
		{
			windowData->drawingOffset.x += drawingZoneOffsetX;
		}
	}
}

void CalculateDrawingZoneSize(WindowData* windowData)
{
	windowData->drawingZone.size.x = windowData->drawingBitmapSize.x;
	windowData->drawingZone.size.y = windowData->drawingBitmapSize.y;

	windowData->drawingZone.UpdateTopRight();

	if (windowData->drawingZone.z > windowData->windowClientSize.x - windowData->drawingZoneCornerResize.size.x)
	{
		windowData->drawingZone.z = windowData->windowClientSize.x - windowData->drawingZoneCornerResize.size.x;
	}

	if (windowData->drawingZone.w > windowData->windowClientSize.y - windowData->drawingZoneCornerResize.size.y)
	{
		windowData->drawingZone.w = windowData->windowClientSize.y - windowData->drawingZoneCornerResize.size.y;
	}
	windowData->drawingZone.UpdateSize();

	windowData->drawingZoneCornerResize.x = windowData->drawingZone.z;
	windowData->drawingZoneCornerResize.y = windowData->drawingZone.w;
	windowData->drawingZoneCornerResize.UpdateTopRight();
}

void FillBitmapWithWhite(ubyte4* bitmap, int2 bitmapSize)
{
	// around 350 microseconds
	memset(bitmap, 255, bitmapSize.x * bitmapSize.y * 4);

	/*int length = bitmapSize.x * bitmapSize.y;
	for (int i = 0; i < length; i++)
	{
		bitmap[i] = ubyte4(255, 255, 255, 0);
	}*/
}

void DrawLine(ubyte4* bitmap, int2 bitmapSize, int4 bitmapRect, int2 from, int2 to, ubyte3 color)
{
	float dx = (float)abs(to.x - from.x);
	float dy = (float)-abs(to.y - from.y);

	char sx = to.x > from.x ? 1 : -1;
	char sy = to.y > from.y ? 1 : -1;

	float error = dy + dx;

	while (from.x != to.x || from.y != to.y)
	{
		if (IsInRect(bitmapRect, from))
		{
			DrawPixel(bitmap, bitmapSize, from, color);
		}

		float e2 = 2 * error;

		if (e2 >= dy)
		{
			error += dy;
			from.x += sx;
		}
		else if (e2 <= dx)
		{
			error += dx;
			from.y += sy;
		}
	}
}

void DrawRect(WindowData* windowData, int x, int y, int width, int height, ubyte3 color)
{
	ubyte4* currentPixel = windowData->windowBitmap + (x + windowData->windowClientSize.x * y);
	int pitch = windowData->windowClientSize.x;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			*currentPixel = ubyte4(color.z, color.y, color.x, 0);
			currentPixel++;
		}

		currentPixel -= width;
		currentPixel += pitch;
	}
}

void DrawBorderRect(WindowData* windowData, int2 bottomLeft, int2 size, int lineWidth, ubyte3 color)
{
	// bottom
	DrawRect(windowData, bottomLeft.x, bottomLeft.y, size.x, lineWidth, color);

	// top
	DrawRect(windowData, bottomLeft.x, bottomLeft.y + size.y - lineWidth, size.x, lineWidth, color);

	// left
	DrawRect(windowData, bottomLeft.x, bottomLeft.y, lineWidth, size.y, color);

	// right
	DrawRect(windowData, bottomLeft.x + size.x - lineWidth, bottomLeft.y, lineWidth, size.y, color);
}

// TODO: current algorithm is to slow
// Bresenham’s algorithm implementation
void FillFromPixel(WindowData* windowData, int2 fromPixel, ubyte3 color)
{
	Queue<int2> queue = Queue<int2>(5);

	ubyte3 previousColor = GetPixelColor(windowData->drawingBitmap, windowData->drawingBitmapSize, fromPixel);

	if (color.x == previousColor.x
		&& color.y == previousColor.y
		&& color.z == previousColor.z)
	{
		return;
	}
	DrawPixel(windowData->drawingBitmap, windowData->drawingBitmapSize, fromPixel, color);

	queue.enqueue(fromPixel);

	int2 neighbourDeltas[] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

	while (!queue.empty())
	{
		fromPixel = queue.pop();
		for (int i = 0; i < ARRAYSIZE(neighbourDeltas); i++)
		{
			int2 neighbourDelta = neighbourDeltas[i];

			int2 neighbourPixel = { fromPixel.x + neighbourDelta.x, fromPixel.y + neighbourDelta.y };

			if (neighbourPixel.x < 0 || neighbourPixel.x >= windowData->drawingBitmapSize.x
				|| neighbourPixel.y < 0 || neighbourPixel.y >= windowData->drawingBitmapSize.y)
			{
				continue;
			}

			ubyte3 previousNeighbourColor = GetPixelColor(windowData->drawingBitmap, windowData->drawingBitmapSize, neighbourPixel);

			if (previousNeighbourColor.x == previousColor.x
				&& previousNeighbourColor.y == previousColor.y
				&& previousNeighbourColor.z == previousColor.z)
			{
				DrawPixel(windowData->drawingBitmap, windowData->drawingBitmapSize, neighbourPixel, color);

				queue.enqueue(neighbourPixel);
			}
		}
	}
}

inline ubyte3 GetPixelColor(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord)
{
	ubyte4 currentPixel = *(bitmap + (pixelCoord.x + bitmapSize.x * pixelCoord.y));

	return {
		currentPixel.x,
		currentPixel.y,
		currentPixel.z
	};
}

inline void DrawPixel(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord, ubyte3 color)
{
	ubyte4* currentPixel = bitmap + (pixelCoord.x + bitmapSize.x * pixelCoord.y);

	*currentPixel = ubyte4(color.z, color.y, color.x, 0);
}

void DrawBitmap(WindowData* windowData, ubyte4* bitmapToCopy, int2 bottomLeft, int2 bitmapSize)
{
	for (int y = 0; y < bitmapSize.y; y++)
	{
		for (int x = 0; x < bitmapSize.x; x++)
		{
			windowData->windowBitmap[bottomLeft.x + x + (bottomLeft.y + y) * windowData->windowClientSize.x]
				= bitmapToCopy[x + y * bitmapSize.x];
		}
	}
}

void CopyBitmapToBitmap(ubyte4* sourceBitmap, int2 sourceBitmapSize,
	ubyte4* destBitmap, int2 destXY, int2 destBitmapSize)
{
	int width = sourceBitmapSize.x;
	int height = sourceBitmapSize.y;

	if (width > destBitmapSize.x - destXY.x)
	{
		width = destBitmapSize.x - destXY.x;
	}

	if (height > destBitmapSize.y - destXY.y)
	{
		height = destBitmapSize.y - destXY.y;
	}

	for (int y = 0; y < height; y++)
	{
		int yDest = destXY.y + y;
		for (int x = 0; x < width; x++)
		{
			ubyte4 sourceColor = sourceBitmap[x + y * sourceBitmapSize.x];

			int xDest = destXY.x + x;

			destBitmap[xDest + yDest * destBitmapSize.x] = sourceColor;
		}
	}
}