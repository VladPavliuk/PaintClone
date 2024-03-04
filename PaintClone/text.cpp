#include "text.h"

void DrawTextLine(WideString* string, int2 bottomLeft, FontDataRasterized* font, ubyte4* bitmap, int2 bitmapSize)
{
	bottomLeft.y -= font->descent;

	for (int i = 0; i < string->length; i++)
	{
		wchar_t symbol = string->chars[i];

		RasterizedGlyph glyph = font->glyphs.get(symbol);
		int2 glyphPosition = { bottomLeft.x, bottomLeft.y + glyph.boundaries.y };

		if (glyph.hasBitmap)
		{
			CopyMonochromicBitmapToBitmap(glyph.bitmap, glyph.bitmapSize, bitmap, glyphPosition, bitmapSize);
		}

		bottomLeft.x += glyph.advanceWidth;
	}
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
