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
	FillBitmapWithWhite(windowData->windowBitmap.pixels, windowData->windowBitmap.size);
}

void InitRenderer(WindowData* windowData, HWND hwnd)
{
	windowData->windowDC = GetDC(hwnd);
	windowData->backgroundDC = CreateCompatibleDC(windowData->windowDC);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	windowData->windowBitmap.size.x = clientRect.right - clientRect.left;
	windowData->windowBitmap.size.y = clientRect.bottom - clientRect.top;

	windowData->windowBitmapInfo.bmiHeader.biSize = sizeof(windowData->windowBitmapInfo.bmiHeader);
	windowData->windowBitmapInfo.bmiHeader.biWidth = windowData->windowBitmap.size.x;
	windowData->windowBitmapInfo.bmiHeader.biHeight = windowData->windowBitmap.size.y;
	windowData->windowBitmapInfo.bmiHeader.biPlanes = 1;
	windowData->windowBitmapInfo.bmiHeader.biBitCount = 32;
	windowData->windowBitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->windowBitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->windowBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->windowBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->windowBitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->windowBitmapInfo.bmiHeader.biClrImportant = 0;

	windowData->drawingBitmapInfo.bmiHeader.biSize = sizeof(windowData->drawingBitmapInfo.bmiHeader);
	windowData->drawingBitmapInfo.bmiHeader.biWidth = windowData->canvasBitmap.size.x;
	windowData->drawingBitmapInfo.bmiHeader.biHeight = windowData->canvasBitmap.size.y;
	windowData->drawingBitmapInfo.bmiHeader.biPlanes = 1;
	windowData->drawingBitmapInfo.bmiHeader.biBitCount = 32;
	windowData->drawingBitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->drawingBitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->drawingBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->drawingBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->drawingBitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->drawingBitmapInfo.bmiHeader.biClrImportant = 0;

	windowData->canvasBitmap.pixels = (ubyte4*)malloc(4 * windowData->canvasBitmap.size.x * windowData->canvasBitmap.size.y);
	FillBitmapWithWhite(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size);

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
		int drawingZoneOffsetY = windowData->drawingZoomLevel * windowData->canvasBitmap.size.y
			- windowData->drawingZone.size().y - windowData->drawingOffset.y;

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
		int drawingZoneOffsetX = windowData->drawingZoomLevel * windowData->canvasBitmap.size.x
			- windowData->drawingZone.size().x - windowData->drawingOffset.x;

		if (drawingZoneOffsetX < 0)
		{
			windowData->drawingOffset.x += drawingZoneOffsetX;
		}
	}
}

void CalculateDrawingZoneSize(WindowData* windowData)
{
	windowData->drawingZone.zw(windowData->drawingZone.xy() + windowData->canvasBitmap.size);
	windowData->drawingZone = ClipRect(windowData->drawingZone, int4({ 0,0 }, windowData->windowBitmap.size - 10));

	windowData->drawingZoneCornerResize = int4(windowData->drawingZone.zw(), windowData->drawingZone.zw() + 10);
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

void CopyTextBufferToCanvas(WindowData* windowData)
{
	int4 textBlockOnCanvas = windowData->textBlockOnClient;

	textBlockOnCanvas.xy(textBlockOnCanvas.xy() - windowData->drawingZone.xy());
	textBlockOnCanvas.zw(textBlockOnCanvas.zw() - windowData->drawingZone.xy());

	textBlockOnCanvas.xy(textBlockOnCanvas.xy() + windowData->drawingOffset);
	textBlockOnCanvas.zw(textBlockOnCanvas.zw() + windowData->drawingOffset);

	textBlockOnCanvas /= (float)windowData->drawingZoomLevel;

	int2 bottomLeft = { windowData->textBlockOnClient.x,
		windowData->textBlockOnClient.w - windowData->fontData.lineHeight * windowData->drawingZoomLevel };
	bottomLeft -= { windowData->drawingZone.x, windowData->drawingZone.y };
	bottomLeft += windowData->drawingOffset;
	bottomLeft /= windowData->drawingZoomLevel;
	int topLineIndex = windowData->topLineIndexToShow;
	int maxLinesInTextBlock = windowData->textBlockOnClient.size().y / windowData->fontData.lineHeight;

	for (int layoutLineIndex = topLineIndex, lineIndex = 0; layoutLineIndex < windowData->glyphsLayout->length; layoutLineIndex++, lineIndex++)
	{
		if (lineIndex >= maxLinesInTextBlock)
		{
			break;
		}

		auto line = windowData->glyphsLayout->get(layoutLineIndex);
		int lineTopOffset = (windowData->textBlockOnClient.w - windowData->drawingZone.y) - (lineIndex + 1) * windowData->fontData.lineHeight;

		for (int j = 0; j < line.length; j++)
		{
			int2 glyphData = line.get(j);
			int charIndex = glyphData.y;

			wchar_t code = windowData->textBuffer.chars[charIndex];
			int lineLeftOffset = (windowData->textBlockOnClient.x - windowData->drawingZone.x) + glyphData.x;

			if (code != L'\n' && code != L'\0') // glyphs layout includes \0 at the end of the last line
			{
				RasterizedGlyph rasterizedGlyph = windowData->fontData.glyphs.get(code);
				int2 position = {
					lineLeftOffset + rasterizedGlyph.leftSideBearings * windowData->drawingZoomLevel,
					lineTopOffset + (rasterizedGlyph.boundaries.y + -windowData->fontData.descent) * windowData->drawingZoomLevel };

				if (rasterizedGlyph.hasBitmap)
				{
					CopyMonochromicBitmapToBitmap(rasterizedGlyph.bitmap, rasterizedGlyph.bitmapSize,
						windowData->canvasBitmap.pixels, position, windowData->canvasBitmap.size);
				}
			}
		}
	}
}

void DrawRect(Bitmap bitmap, int2 bottomLeft, int2 size, ubyte3 color)
{
	ubyte4* currentPixel = bitmap.pixels + (bottomLeft.x + bitmap.size.x * bottomLeft.y);
	int pitch = bitmap.size.x;

	for (int i = 0; i < size.y; i++)
	{
		for (int j = 0; j < size.x; j++)
		{
			*currentPixel = ubyte4(color.z, color.y, color.x, 0);
			currentPixel++;
		}

		currentPixel -= size.x;
		currentPixel += pitch;
	}
}

void DrawBorderRect(Bitmap bitmap, int2 bottomLeft, int2 size, int lineWidth, ubyte3 color)
{
	// bottom
	DrawRect(bitmap, bottomLeft, { size.x, lineWidth }, color);

	// top
	DrawRect(bitmap, { bottomLeft.x, bottomLeft.y + size.y - lineWidth }, { size.x, lineWidth }, color);

	// left
	DrawRect(bitmap, bottomLeft, { lineWidth, size.y }, color);

	// right
	DrawRect(bitmap, { bottomLeft.x + size.x - lineWidth, bottomLeft.y }, { lineWidth, size.y }, color);
}

// TODO: current algorithm is to slow
// Bresenham’s algorithm implementation
void FillFromPixel(WindowData* windowData, int2 fromPixel, ubyte3 color)
{
	Queue<int2> queue = Queue<int2>(5);

	ubyte3 previousColor = GetPixelColor(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, fromPixel);

	if (color.x == previousColor.x
		&& color.y == previousColor.y
		&& color.z == previousColor.z)
	{
		return;
	}
	DrawPixel(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, fromPixel, color);

	queue.enqueue(fromPixel);

	int2 neighbourDeltas[] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

	while (!queue.empty())
	{
		fromPixel = queue.pop();
		for (int i = 0; i < ARRAYSIZE(neighbourDeltas); i++)
		{
			int2 neighbourDelta = neighbourDeltas[i];

			int2 neighbourPixel = { fromPixel.x + neighbourDelta.x, fromPixel.y + neighbourDelta.y };

			if (neighbourPixel.x < 0 || neighbourPixel.x >= windowData->canvasBitmap.size.x
				|| neighbourPixel.y < 0 || neighbourPixel.y >= windowData->canvasBitmap.size.y)
			{
				continue;
			}

			ubyte3 previousNeighbourColor = GetPixelColor(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, neighbourPixel);

			if (previousNeighbourColor.x == previousColor.x
				&& previousNeighbourColor.y == previousColor.y
				&& previousNeighbourColor.z == previousColor.z)
			{
				DrawPixel(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, neighbourPixel, color);

				queue.enqueue(neighbourPixel);
			}
		}
	}
}

inline ubyte3 GetPixelColor(ubyte4* bitmap, int2 bitmapSize, int2 pixelCoord)
{
	ubyte4 currentPixel = *(bitmap + (pixelCoord.x + bitmapSize.x * pixelCoord.y));

	return {
		currentPixel.z,
		currentPixel.y,
		currentPixel.x
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
			windowData->windowBitmap.pixels[bottomLeft.x + x + (bottomLeft.y + y) * windowData->windowBitmap.size.x]
				= bitmapToCopy[x + y * bitmapSize.x];
		}
	}
}

void CopyMonochromicBitmapToBitmap(ubyte* sourceBitmap, int2 sourceBitmapSize,
	ubyte4* destBitmap, int2 destXY, int2 destBitmapSize, int zoom, bool invertColor)
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

	int colorMask = invertColor ? 0 : 255;
	for (int y = 0; y < height; y++)
	{
		int yDest = destXY.y + y * zoom;
		for (int x = 0; x < width; x++)
		{
			ubyte sourceColor = invertColor
				? sourceBitmap[x + y * sourceBitmapSize.x]
				: (colorMask - sourceBitmap[x + y * sourceBitmapSize.x]);

			if (sourceColor == colorMask) continue;

			int xDest = destXY.x + x * zoom;

			for (int innerY = 0; innerY < zoom; innerY++)
			{
				for (int innerX = 0; innerX < zoom; innerX++)
				{
					destBitmap[innerX + xDest + (innerY + yDest) * destBitmapSize.x] = { sourceColor, sourceColor, sourceColor, sourceColor };
				}
			}
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
