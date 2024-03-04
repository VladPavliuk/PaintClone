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
		int drawingZoneOffsetX = windowData->drawingZoomLevel * windowData->drawingBitmapSize.x
			- windowData->drawingZone.size().x - windowData->drawingOffset.x;

		if (drawingZoneOffsetX < 0)
		{
			windowData->drawingOffset.x += drawingZoneOffsetX;
		}
	}
}

void CalculateDrawingZoneSize(WindowData* windowData)
{
	windowData->drawingZone.zw(windowData->drawingZone.xy() + windowData->drawingBitmapSize);
	windowData->drawingZone = ClipRect(windowData->drawingZone, int4({ 0,0 }, windowData->windowClientSize - 10));

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

	textBlockOnCanvas /= windowData->drawingZoomLevel;

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
						windowData->drawingBitmap, position, windowData->drawingBitmapSize);
				}
			}
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

int4 ClipRect(int4 rectSource, int4 rectDest)
{
	int4 clippedRect = rectSource;

	if (rectSource.x < rectDest.x) clippedRect.x = rectDest.x;
	if (rectSource.y < rectDest.y) clippedRect.y = rectDest.y;

	if (rectSource.z > rectDest.z) clippedRect.z = rectDest.z;
	if (rectSource.w > rectDest.w) clippedRect.w = rectDest.w;

	return clippedRect;
}

int4 ClipRect(int4 rectSource, int2 rectDest)
{
	return ClipRect(rectSource, { 0, 0, rectDest.x, rectDest.y });
}

int2 ClipPoint(int2 point, int4 rect)
{
	int2 clippedPoint = point;

	if (point.x < rect.x) clippedPoint.x = rect.x;
	else if (point.x > rect.z) clippedPoint.x = rect.z;

	if (point.y < rect.y) clippedPoint.y = rect.y;
	else if (point.y > rect.w) clippedPoint.y = rect.w;

	return clippedPoint;
}

int ClipPoint(int point, int2 range)
{
	int clippedPoint = point;

	if (point < range.x) clippedPoint = range.x;
	else if (point > range.y) clippedPoint = range.y;

	return clippedPoint;
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
			windowData->windowBitmap[bottomLeft.x + x + (bottomLeft.y + y) * windowData->windowClientSize.x]
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

// TODO: that's not the most quickest way to get cursor location at runtime
// better approach would be to cache it after some actions
int2 GetCursorLayoutPotision(WindowData* windowData)
{
	for (int layoutLineIndex = 0; layoutLineIndex < windowData->glyphsLayout->length; layoutLineIndex++)
	{
		auto layoutLine = windowData->glyphsLayout->get(layoutLineIndex);

		for (int j = 0; j < layoutLine.length; j++)
		{
			int2 symbol = layoutLine.get(j);

			if (symbol.y == windowData->cursorPosition)
			{
				return { layoutLineIndex, symbol.x };
			}
		}
	}

	int lastLineIndex = windowData->glyphsLayout->length - 1;
	auto lastLine = windowData->glyphsLayout->get(lastLineIndex);

	int2 lastSymbol = lastLine.get(lastLine.length - 1);
	return { lastLineIndex, lastSymbol.x };
}

int GetCursorPositionByMousePosition(WindowData* windowData)
{
	int2 mousePosition = windowData->mousePosition;

	// project mouse position on screen on text block where text is being rendered
	int2 projectedMousePosition = { -1, -1 };

	if (windowData->textBlockOnClient.x > mousePosition.x)
	{
		projectedMousePosition.x = windowData->textBlockOnClient.x;
	}
	else if (mousePosition.x > windowData->textBlockOnClient.z)
	{
		projectedMousePosition.x = windowData->textBlockOnClient.z;
	}
	else
	{
		projectedMousePosition.x = mousePosition.x;
	}

	if (windowData->textBlockOnClient.y > mousePosition.y)
	{
		projectedMousePosition.y = windowData->textBlockOnClient.y + 1;
	}
	else if (mousePosition.y > windowData->textBlockOnClient.w)
	{
		projectedMousePosition.y = windowData->textBlockOnClient.w - 1;
	}
	else
	{
		projectedMousePosition.y = mousePosition.y;
	}

	projectedMousePosition.x -= windowData->textBlockOnClient.x;
	projectedMousePosition.y -= windowData->textBlockOnClient.y;

	int textBlockHeight = windowData->textBlockOnClient.size().y;
	int lineHeight = windowData->fontData.lineHeight;
	int projectedLineIndex = ((textBlockHeight - projectedMousePosition.y) / lineHeight);

	projectedLineIndex += windowData->topLineIndexToShow;

	if (projectedLineIndex >= windowData->glyphsLayout->length)
	{
		int lastLindeIndex = windowData->glyphsLayout->length - 1;
		auto lastLine = windowData->glyphsLayout->get(lastLindeIndex);
		int2 symbol = lastLine.get(lastLine.length - 1);
		return symbol.y;
	}

	auto line = windowData->glyphsLayout->get(projectedLineIndex);

	int textIndex = -1;
	for (int i = 0; i < line.length; i++)
	{
		int2 symbol = line.get(i);

		if (projectedMousePosition.x < symbol.x)
		{
			bool hasPrevSymbol = i > 0;
			if (hasPrevSymbol)
			{
				int2 prevSymbol = line.get(i - 1);

				if ((symbol.x - projectedMousePosition.x) < (projectedMousePosition.x - prevSymbol.x))
				{
					textIndex = symbol.y;
				}
			}
			break;
		}

		textIndex = symbol.y;
	}

	return textIndex;
}

void _moveToNextLine(WindowData* windowData,
	int lineHeight, int textBlockHeight,
	int* currentLineWidth, int* lineIndex)
{
	*currentLineWidth = 0;
	*lineIndex = *lineIndex + 1;
	windowData->glyphsLayout->add(SimpleDynamicArray<int2>(1));

	if (lineHeight * (*lineIndex + 1) > textBlockHeight)
	{
		windowData->textBlockOnClient.y -= lineHeight;

		if (windowData->textBlockOnClient.y < windowData->drawingZone.y)
			windowData->textBlockOnClient.y = windowData->drawingZone.y;
	}
}

void UpdateTextBlockTopLine(WindowData* windowData)
{
	int2 cursorLayoutPosition = GetCursorLayoutPotision(windowData);
	int cursorLineIndex = cursorLayoutPosition.x;
	int maxLinesInTextBlock = windowData->textBlockOnClient.size().y / windowData->fontData.lineHeight;

	if (cursorLineIndex < windowData->topLineIndexToShow)
	{
		windowData->topLineIndexToShow = cursorLineIndex;
	}
	else if (cursorLineIndex >= windowData->topLineIndexToShow + maxLinesInTextBlock)
	{
		windowData->topLineIndexToShow = cursorLineIndex - maxLinesInTextBlock + 1;
	}
}

void UpdateTextSelectionifShiftPressed(WindowData* windowData)
{
	//TODO: it's better no to work with keystate directly, create some middle staff
	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		if (windowData->selectedTextStartIndex != -1)
		{
			return;
		}

		windowData->selectedTextStartIndex = windowData->cursorPosition;
		return;
	}

	windowData->selectedTextStartIndex = -1;
}

void MoveCursorToNewLine(WindowData* windowData, int newLineIndex, int oldCursorLeftOffset)
{
	if (newLineIndex < 0 || newLineIndex >= windowData->glyphsLayout->length) return;

	auto line = windowData->glyphsLayout->get(newLineIndex);

	//TODO: copypasta from mouse click logic, move it a function
	for (int i = 0; i < line.length; i++)
	{
		int2 symbol = line.get(i);

		if (oldCursorLeftOffset < symbol.x)
		{
			bool hasPrevSymbol = i > 0;
			if (hasPrevSymbol)
			{
				int2 prevSymbol = line.get(i - 1);

				if ((symbol.x - oldCursorLeftOffset) < (oldCursorLeftOffset - prevSymbol.x))
				{
					windowData->cursorPosition = symbol.y;
				}
			}
			break;
		}

		windowData->cursorPosition = symbol.y;
	}
}

int2 GetSelectedTextRange(WindowData* windowData)
{
	int fromIndex = minInt(windowData->cursorPosition, windowData->selectedTextStartIndex);
	int toIndex = maxInt(windowData->cursorPosition, windowData->selectedTextStartIndex);

	return { fromIndex, toIndex };
}

// NOTE: it is too slow in theory
void RecreateGlyphsLayout(WindowData* windowData, WideString text, int lineMaxWidth)
{
	if (windowData->glyphsLayout != NULL)
	{
		for (int i = 0; i < windowData->glyphsLayout->length; i++)
		{
			windowData->glyphsLayout->get(i).freeMemory();
		}
		windowData->glyphsLayout->freeMemory();
	}

	windowData->glyphsLayout = SimpleDynamicArray<SimpleDynamicArray<int2>>::allocate(1);
	windowData->glyphsLayout->add(SimpleDynamicArray<int2>(1));
	
	int scale = windowData->drawingZoomLevel;
	int textBlockHeight = windowData->textBlockOnClient.size().y * scale;
	int lineHeight = windowData->fontData.lineHeight * scale;

	int currentLineWidth = 0; // in pixels
	int lineIndex = 0;
	int i = 0;
	for (i = 0; i < text.length; i++)
	{
		if (text.chars[i] == L'\n')
		{
			windowData->glyphsLayout->getPointer(lineIndex)->add({ currentLineWidth, i });

			_moveToNextLine(windowData, lineHeight, textBlockHeight,
				&currentLineWidth, &lineIndex);

			continue;
		}

		RasterizedGlyph glyph = windowData->fontData.glyphs.get(text.chars[i]);

		windowData->glyphsLayout->getPointer(lineIndex)->add({ currentLineWidth, i });

		currentLineWidth += glyph.advanceWidth * scale;

		bool hasNextSymbol = i < text.length - 1;

		if (hasNextSymbol && text.chars[i + 1] != L'\n')
		{
			RasterizedGlyph nextGlyph = windowData->fontData.glyphs.get(text.chars[i + 1]);

			if (nextGlyph.advanceWidth * scale + currentLineWidth > lineMaxWidth)
			{
				_moveToNextLine(windowData, lineHeight, textBlockHeight,
					&currentLineWidth, &lineIndex);
			}
		}
	}

	// NOTE: We add the last char in the string (\0), for correct cursor work
	windowData->glyphsLayout->getPointer(lineIndex)->add({ currentLineWidth, i });
}
