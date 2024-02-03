#include "renderer.h"

void InitRenderer(WindowData* windowData, HWND hwnd)
{
	windowData->deviceContext = GetDC(hwnd);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	windowData->clientSize.x = clientRect.right - clientRect.left;
	windowData->clientSize.y = clientRect.bottom - clientRect.top;

	windowData->bitmapInfo.bmiHeader.biSize = sizeof(windowData->bitmapInfo.bmiHeader);
	windowData->bitmapInfo.bmiHeader.biWidth = windowData->clientSize.x;
	windowData->bitmapInfo.bmiHeader.biHeight = windowData->clientSize.y;
	windowData->bitmapInfo.bmiHeader.biPlanes = 1;
	windowData->bitmapInfo.bmiHeader.biBitCount = 32;
	windowData->bitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->bitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->bitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->bitmapInfo.bmiHeader.biClrImportant = 0;
	//windowData->bitmapInfo.bmiColors = 0;

	// create actual bitmap with r,g,b,_
	windowData->bitmap = (ubyte4*)malloc(4 * windowData->clientSize.x * windowData->clientSize.y);
	//ZeroMemory(windowData->bitmap, 4 * windowData->clientSize.x * windowData->clientSize.y);

	windowData->zAndIdBuffer = (ubyte2*)malloc(2 * windowData->clientSize.x * windowData->clientSize.y);
	ZeroMemory(windowData->zAndIdBuffer, 2 * windowData->clientSize.x * windowData->clientSize.y);
}

void FillWindowClientWithWhite(WindowData* windowData)
{
	int length = windowData->clientSize.x * windowData->clientSize.y;
	for (int i = 0; i < length; i++)
	{
		windowData->bitmap[i] = ubyte4(255, 255, 255, 0);
	}
}

void DrawLine(WindowData* windowData, int2 from, int2 to, ubyte3 color)
{
	float dx = abs(to.x - from.x);
	float dy = -abs(to.y - from.y);

	char sx = to.x > from.x ? 1 : -1;
	char sy = to.y > from.y ? 1 : -1;

	float error = dy + dx;

	while (from.x != to.x || from.y != to.y)
	{
		DrawPixel(windowData, from.x, from.y, color);
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
	ubyte4* currentPixel = windowData->bitmap + (x + windowData->clientSize.x * y);
	int pitch = windowData->clientSize.x;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			*currentPixel = ubyte4(color.z, color.y, color.x, 0);
			//currentPixel++;

			//*currentPixel = color.y; //g
			//currentPixel++;

			//*currentPixel = color.x; // r
			//currentPixel++;

			//*currentPixel = 0;
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

	ubyte3 previousColor = GetPixelColor(windowData, fromPixel.x, fromPixel.y);

	if (color.x == previousColor.x
		&& color.y == previousColor.y
		&& color.z == previousColor.z)
	{
		return;
	}
	DrawPixel(windowData, fromPixel.x, fromPixel.y, color);

	queue.enqueue(fromPixel);

	int2 neighbourDeltas[] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

	while (!queue.empty())
	{
		fromPixel = queue.pop();
		for (int i = 0; i < ARRAYSIZE(neighbourDeltas); i++)
		{
			int2 neighbourDelta = neighbourDeltas[i];

			int2 neighbourPixel = { fromPixel.x + neighbourDelta.x, fromPixel.y + neighbourDelta.y };

			if (neighbourPixel.x < 0 || neighbourPixel.x >= windowData->clientSize.x
				|| neighbourPixel.y < 0 || neighbourPixel.y >= windowData->clientSize.y)
			{
				continue;
			}

			ubyte3 previousNeighbourColor = GetPixelColor(windowData, neighbourPixel.x, neighbourPixel.y);

			if (previousNeighbourColor.x == previousColor.x
				&& previousNeighbourColor.y == previousColor.y
				&& previousNeighbourColor.z == previousColor.z)
			{
				DrawPixel(windowData, neighbourPixel.x, neighbourPixel.y, color);

				queue.enqueue(neighbourPixel);
			}
		}
	}
}

inline ubyte3 GetPixelColor(WindowData* windowData, int x, int y)
{
	ubyte4 currentPixel = *(windowData->bitmap + (x + windowData->clientSize.x * y));

	return {
		currentPixel.x,
		currentPixel.y,
		currentPixel.z
	};
}

inline void DrawPixel(WindowData* windowData, int x, int y, ubyte3 color)
{
	ubyte4* currentPixel = windowData->bitmap + (x + windowData->clientSize.x * y);

	*currentPixel = ubyte4(color.z, color.y, color.x, 0);
}

void DrawBitmap(WindowData* windowData, ubyte4* bitmapToCopy, int2 topLeft, int2 bitmapSize)
{
	for (int y = 0; y < bitmapSize.y; y++)
	{
		for (int x = 0; x < bitmapSize.x; x++)
		{
			windowData->bitmap[topLeft.x + x + (topLeft.y + y) * windowData->clientSize.x] 
				= bitmapToCopy[x + y * bitmapSize.x];
		}
	}
}