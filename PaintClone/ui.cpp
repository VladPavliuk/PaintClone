#include "ui.h"

void HandleUiElements(WindowData* windowData)
{
	switch (windowData->sumbitedUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	/*case UI_ELEMENT::TEXT_BLOCK:
	{
		break;
	}*/
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		switch (windowData->selectedTool)
		{
		case DRAW_TOOL::FILL:
		{
			int2 pixelToStart = windowData->mousePosition;

			pixelToStart = ConvertFromScreenToDrawingCoords(windowData, pixelToStart);

			FillFromPixel(windowData, pixelToStart, windowData->selectedColor);
			break;
		}
		case DRAW_TOOL::TEXT:
		{
			// if mode off, create text block
			//		if mode on and click in box ...
			// if mode on and click outside box, create empty box
			
			//> clear previuos state of text block
			if (windowData->textBuffer.length > 0)
			{
				CopyTextBufferToCanvas(windowData);
			}

			windowData->textBlockOnClient = { -1,-1,-1,-1 };
			windowData->cursorPosition = -1;
			windowData->topLineIndexToShow = 0;
			windowData->selectedTextStartIndex = -1;
			windowData->textBuffer.clear();
			//<

			// create text block state
			int defaultLinesPerBlock = 3;
			int defaultTextBlockWidth = 100 * windowData->drawingZoomLevel;
			int defaultTextBlockHeight = defaultLinesPerBlock * windowData->fontData.lineHeight * windowData->drawingZoomLevel;

			int4 textBlockRect;
			textBlockRect.x = windowData->mousePosition.x;
			textBlockRect.y = windowData->mousePosition.y;
			textBlockRect.z = minInt(textBlockRect.x + defaultTextBlockWidth, windowData->drawingZone.z);
			textBlockRect.w = minInt(textBlockRect.y + defaultTextBlockHeight, windowData->drawingZone.w);

			int test = (textBlockRect.x - windowData->drawingZone.x) % windowData->drawingZoomLevel;
			int test2 = (textBlockRect.y - windowData->drawingZone.y) % windowData->drawingZoomLevel;

			textBlockRect.x -= test;
			textBlockRect.y -= test2;
			textBlockRect.z -= test;
			textBlockRect.w -= test2;

			windowData->textBlockOnClient = textBlockRect;

			RecreateGlyphsLayout(windowData, windowData->textBuffer, textBlockRect.size().x);

			windowData->cursorPosition = 0;

			windowData->isTextEnteringMode = true;
			break;
		}
		}
		break;
	}
	case UI_ELEMENT::PENCIL_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::PENCIL;
		break;
	}
	case UI_ELEMENT::FILL_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::FILL;
		break;
	}
	case UI_ELEMENT::ZOOM_TOOL:
	{
		windowData->drawingZoomLevel *= 2;

		if (windowData->drawingZoomLevel >= 16)
		{
			windowData->drawingZoomLevel = 1;
		}

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::TEXT_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::TEXT;
		break;
	}
	case UI_ELEMENT::COLOR_BRUCH_1:
	case UI_ELEMENT::COLOR_BRUCH_2:
	case UI_ELEMENT::COLOR_BRUCH_3:
	case UI_ELEMENT::COLOR_BRUCH_4:
	case UI_ELEMENT::COLOR_BRUCH_5:
	case UI_ELEMENT::COLOR_BRUCH_6:
	case UI_ELEMENT::COLOR_BRUCH_7:
	case UI_ELEMENT::COLOR_BRUCH_8:
	{
		for (int i = 0; i < windowData->brushColorTiles.length; i++)
		{
			BrushColorTile brushColorTile = windowData->brushColorTiles.get(i);
			if (brushColorTile.uiElement == windowData->sumbitedUi)
			{
				windowData->selectedColor = brushColorTile.color;
				break;
			}
		}
		break;
	}
	default:
	{
		//OutputDebugString(L"YEAH\n");
		break;
	}
	}
	
	switch (windowData->sumbitedOnAnyHotUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	case UI_ELEMENT::CANVAS_CORNER_RESIZE:
	{
		// recalculate new drawing bitmap size
		int2 previousBitmapSize = windowData->drawingBitmapSize;
		windowData->drawingBitmapSize = windowData->drawingZoneCornerResize.xy() - windowData->drawingZone.xy();

		CalculateDrawingZoneSize(windowData);

		// reallocate drawing bitmap for new drawing size
		ubyte4* previousBitmap = windowData->drawingBitmap;

		windowData->drawingBitmapInfo.bmiHeader.biWidth = windowData->drawingBitmapSize.x;
		windowData->drawingBitmapInfo.bmiHeader.biHeight = windowData->drawingBitmapSize.y;

		windowData->drawingBitmap = (ubyte4*)malloc(4 * windowData->drawingBitmapSize.x * windowData->drawingBitmapSize.y);
		FillBitmapWithWhite(windowData->drawingBitmap, windowData->drawingBitmapSize);

		CopyBitmapToBitmap(previousBitmap, previousBitmapSize,
			windowData->drawingBitmap, { 0,0 }, windowData->drawingBitmapSize);

		free(previousBitmap);

		// adjust canvas scrolls
		ValidateDrawingOffset(windowData);
		break;
	}
	}

	switch (windowData->activeUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	case UI_ELEMENT::CANVAS_CORNER_RESIZE:
	{
		int2 drawingZoneCornerResizeSize = windowData->drawingZoneCornerResize.size();

		windowData->drawingZoneCornerResize.xy(windowData->mousePosition - windowData->activeUiOffset);

		windowData->drawingZoneCornerResize.zw(windowData->drawingZoneCornerResize.xy() + drawingZoneCornerResizeSize);

		if (windowData->drawingZoneCornerResize.z > windowData->windowClientSize.x)
		{
			windowData->drawingZoneCornerResize.x = windowData->windowClientSize.x - windowData->drawingZoneCornerResize.size().x;
		}
		else if (windowData->drawingZoneCornerResize.x < windowData->drawingZone.x + 1)
		{
			// add 1 because after resize drawing zone should be at least 1 pixel wide
			windowData->drawingZoneCornerResize.x = windowData->drawingZone.x + 1;
		}

		if (windowData->drawingZoneCornerResize.w > windowData->windowClientSize.y)
		{
			windowData->drawingZoneCornerResize.y = windowData->windowClientSize.y - windowData->drawingZoneCornerResize.size().y;
		}
		else if (windowData->drawingZoneCornerResize.y < windowData->drawingZone.y + 1)
		{
			// add 1 because after resize drawing zone should be at least 1 pixel height
			windowData->drawingZoneCornerResize.y = windowData->drawingZone.y + 1;
		}

		windowData->drawingZoneCornerResize.zw(windowData->drawingZoneCornerResize.xy() + drawingZoneCornerResizeSize);
		break;
	}
	case UI_ELEMENT::CANVAS_VERTICAL_SCROLL:
	{
		windowData->drawingOffset.y += windowData->drawingZoomLevel * (windowData->mousePosition.y - windowData->prevMousePosition.y);

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::CANVAS_HORIZONTAL_SCROLL:
	{
		windowData->drawingOffset.x += windowData->drawingZoomLevel * (windowData->mousePosition.x - windowData->prevMousePosition.x);

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		if (windowData->selectedTool == DRAW_TOOL::PENCIL)
		{
			int2 fromPixel = windowData->prevMousePosition;
			int2 toPixel = windowData->mousePosition;

			if (!IsInRect(windowData->drawingZone, fromPixel) && !IsInRect(windowData->drawingZone, toPixel))
			{
				break;
			}

			fromPixel = ConvertFromScreenToDrawingCoords(windowData, fromPixel);
			toPixel = ConvertFromScreenToDrawingCoords(windowData, toPixel);

			int4 drawingRect;

			drawingRect.x = 0;
			drawingRect.y = 0;
			drawingRect.z = windowData->drawingZone.size().x;
			drawingRect.w = windowData->drawingZone.size().y;

			drawingRect.x += windowData->drawingOffset.x;
			drawingRect.y += windowData->drawingOffset.y;
			drawingRect.z += windowData->drawingOffset.x;
			drawingRect.w += windowData->drawingOffset.y;

			drawingRect.x = (int)((float)drawingRect.x / (float)windowData->drawingZoomLevel);
			drawingRect.y = (int)((float)drawingRect.y / (float)windowData->drawingZoomLevel);
			drawingRect.z = (int)((float)drawingRect.z / (float)windowData->drawingZoomLevel);
			drawingRect.w = (int)((float)drawingRect.w / (float)windowData->drawingZoomLevel);

			if (fromPixel.x == toPixel.x && fromPixel.y == toPixel.y)
			{
				DrawPixel(windowData->drawingBitmap, windowData->drawingBitmapSize, toPixel, windowData->selectedColor);
			}
			else
			{
				DrawLine(windowData->drawingBitmap, windowData->drawingBitmapSize, drawingRect, fromPixel, toPixel, windowData->selectedColor);
			}
		}

		break;
	}
	case UI_ELEMENT::TEXT_BLOCK:
	{
		int oldCursorPosition = windowData->cursorPosition;
		windowData->cursorPosition = GetCursorPositionByMousePosition(windowData);
		
		if (windowData->wasRightButtonPressed)
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				windowData->selectedTextStartIndex = oldCursorPosition;
			}
			else
			{
				windowData->selectedTextStartIndex = windowData->cursorPosition;
			}
		}
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_TOP_LEFT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.x = windowData->mousePosition.x + windowData->activeUiOffset.x;

		if (windowData->textBlockOnClient.x > windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.x = windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x;
		}
		else if (windowData->textBlockOnClient.x < windowData->drawingZone.x + windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.x = windowData->drawingZone.x + windowData->textBlockButtonsSize.x;
		}

		windowData->textBlockOnClient.w = windowData->mousePosition.y - windowData->activeUiOffset.y;

		if (windowData->textBlockOnClient.w < windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.w = windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y;
		}
		else if (windowData->textBlockOnClient.w > windowData->drawingZone.w - windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.w = windowData->drawingZone.w - windowData->textBlockButtonsSize.y;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_TOP_RIGHT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.z = windowData->mousePosition.x - windowData->activeUiOffset.x;

		if (windowData->textBlockOnClient.z > windowData->drawingZone.z - windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.z = windowData->drawingZone.z - windowData->textBlockButtonsSize.x;
		}
		else if (windowData->textBlockOnClient.z < windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.z = windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x;
		}

		windowData->textBlockOnClient.w = windowData->mousePosition.y - windowData->activeUiOffset.y;

		if (windowData->textBlockOnClient.w < windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.w = windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y;
		}
		else if (windowData->textBlockOnClient.w > windowData->drawingZone.w - windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.w = windowData->drawingZone.w - windowData->textBlockButtonsSize.y;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_BOTTOM_RIGHT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.z = windowData->mousePosition.x - windowData->activeUiOffset.x;

		if (windowData->textBlockOnClient.z > windowData->drawingZone.z - windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.z = windowData->drawingZone.z - windowData->textBlockButtonsSize.x;
		}
		else if (windowData->textBlockOnClient.z < windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.z = windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x;
		}

		windowData->textBlockOnClient.y = windowData->mousePosition.y + windowData->activeUiOffset.y;

		if (windowData->textBlockOnClient.y < windowData->drawingZone.y + windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.y = windowData->drawingZone.y + windowData->textBlockButtonsSize.y;
		}
		else if (windowData->textBlockOnClient.y > windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.y = windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_BOTTOM_LEFT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.x = windowData->mousePosition.x + windowData->activeUiOffset.x;

		if (windowData->textBlockOnClient.x > windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.x = windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x;
		}
		else if (windowData->textBlockOnClient.x < windowData->drawingZone.x + windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.x = windowData->drawingZone.x + windowData->textBlockButtonsSize.x;
		}

		windowData->textBlockOnClient.y = windowData->mousePosition.y + windowData->activeUiOffset.y;

		if (windowData->textBlockOnClient.y < windowData->drawingZone.y + windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.y = windowData->drawingZone.y + windowData->textBlockButtonsSize.y;
		}
		else if (windowData->textBlockOnClient.y > windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.y = windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_TOP_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.w = windowData->mousePosition.y - windowData->activeUiOffset.y;

		if (windowData->textBlockOnClient.w < windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.w = windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y;
		}
		else if (windowData->textBlockOnClient.w > windowData->drawingZone.w - windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.w = windowData->drawingZone.w - windowData->textBlockButtonsSize.y;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_RIGHT_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.z = windowData->mousePosition.x - windowData->activeUiOffset.x;

		if (windowData->textBlockOnClient.z > windowData->drawingZone.z - windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.z = windowData->drawingZone.z - windowData->textBlockButtonsSize.x;
		}
		else if (windowData->textBlockOnClient.z < windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.z = windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_BOTTOM_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.y = windowData->mousePosition.y + windowData->activeUiOffset.y;

		if (windowData->textBlockOnClient.y < windowData->drawingZone.y + windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.y = windowData->drawingZone.y + windowData->textBlockButtonsSize.y;
		}
		else if (windowData->textBlockOnClient.y > windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y)
		{
			windowData->textBlockOnClient.y = windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_LEFT_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		windowData->textBlockOnClient.x = windowData->mousePosition.x + windowData->activeUiOffset.x;

		if (windowData->textBlockOnClient.x > windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.x = windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x;
		}
		else if (windowData->textBlockOnClient.x < windowData->drawingZone.x + windowData->textBlockButtonsSize.x)
		{
			windowData->textBlockOnClient.x = windowData->drawingZone.x + windowData->textBlockButtonsSize.x;
		}

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	}
}

void DrawDrawingCanvas(WindowData* windowData)
{
	DrawBorderRect(windowData, { windowData->drawingZone.x, windowData->drawingZone.y },
		windowData->drawingZone.size(), 2, { 255, 0, 0 });
	DrawScrollsForDrawingZone(windowData);

	CheckHotActiveForUiElement(windowData, {
		windowData->drawingZone.x, windowData->drawingZone.y,
		windowData->drawingZone.z, windowData->drawingZone.w
		}, UI_ELEMENT::DRAWING_CANVAS);
}

void DrawScrollsForDrawingZone(WindowData* windowData)
{
	int scrollWidth = 10;
	ubyte3 scrollBgColor = { 150, 150, 150 };
	ubyte3 scrollHoveredBgColor = { 120, 120, 120 };

	// vertical bar
	float drawingZoneToImageHeightRatio = (float)windowData->drawingZone.size().y
		/ ((float)windowData->drawingBitmapSize.y * (float)windowData->drawingZoomLevel);

	if (drawingZoneToImageHeightRatio < 1.0f)
	{
		DrawButton(windowData,
			{
				windowData->drawingZone.x - scrollWidth,
				(int)(drawingZoneToImageHeightRatio * windowData->drawingOffset.y) + windowData->drawingZone.y
			},
			{ scrollWidth, (int)(drawingZoneToImageHeightRatio * windowData->drawingZone.size().y) },
			scrollBgColor, scrollHoveredBgColor, UI_ELEMENT::CANVAS_VERTICAL_SCROLL);
	}

	// horizontal bar
	float drawingZoneToImageWidthRatio = (float)windowData->drawingZone.size().x / ((float)windowData->drawingBitmapSize.x * (float)windowData->drawingZoomLevel);

	if (drawingZoneToImageWidthRatio < 1.0f)
	{
		DrawButton(windowData,
			{
				(int)(drawingZoneToImageWidthRatio * windowData->drawingOffset.x) + windowData->drawingZone.x,
				windowData->drawingZone.y - scrollWidth,
			},
			{ (int)(drawingZoneToImageWidthRatio * windowData->drawingZone.size().x), scrollWidth },
			scrollBgColor, scrollHoveredBgColor, UI_ELEMENT::CANVAS_HORIZONTAL_SCROLL);
	}
}

void DrawPanel(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte3 bgColor,
	ubyte zIndex, ubyte uiId)
{
	/*DrawPanel(windowData,
		bottomLeft, size,
		bgColor,
		zIndex, uiId);

	DrawRectToZAndIdBuffer(windowData,
		bottomLeft, size,
		zIndex, uiId);*/
}

void DrawBitmapButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte4* bitmap,
	ubyte3 hoveredBgColor,
	UI_ELEMENT uiElement)
{
	CheckHotActiveForUiElement(windowData, { bottomLeft.x, bottomLeft.y, bottomLeft.x + size.x, bottomLeft.y + size.y }, uiElement);

	//TODO: add some kind of image stretching because we might want to have buttons that dont have the same size as a bitmap
	DrawBitmap(windowData, bitmap, bottomLeft, size);

	DrawBorderRect(windowData, bottomLeft, size, 1, { 0,0,0 });
}

void DrawButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte3 bgColor, ubyte3 hoveredBgColor,
	UI_ELEMENT uiElement)
{
	CheckHotActiveForUiElement(windowData, { bottomLeft.x, bottomLeft.y, bottomLeft.x + size.x, bottomLeft.y + size.y }, uiElement);

	if (windowData->activeUi == uiElement || (windowData->hotUi == uiElement && windowData->activeUi == UI_ELEMENT::NONE))
	{
		DrawRect(windowData, bottomLeft.x, bottomLeft.y, size.x, size.y, hoveredBgColor);
	}
	else
	{
		DrawRect(windowData, bottomLeft.x, bottomLeft.y, size.x, size.y, bgColor);
	}

	DrawBorderRect(windowData, bottomLeft, size, 1, { 0,0,0 });
}

void DrawColorsBrush(WindowData* windowData, SimpleDynamicArray<BrushColorTile>* brushColors, int2 bottomLeft,
	int2 singleColorTileSize, int xDistanceToNextColor)
{
	for (int i = 0; i < brushColors->length; i++)
	{
		BrushColorTile brushColor = brushColors->get(i);

		DrawButton(windowData,
			{ bottomLeft.x + (singleColorTileSize.x * i) + (xDistanceToNextColor * i), bottomLeft.y },
			singleColorTileSize, brushColor.color, brushColor.color, brushColor.uiElement);

		if (brushColor.color == windowData->selectedColor)
		{
			DrawBorderRect(windowData,
				{ bottomLeft.x + (singleColorTileSize.x * i) + (xDistanceToNextColor * i) - 1, bottomLeft.y - 1 },
				{ singleColorTileSize.x + 1, singleColorTileSize.y + 1 }, 2, { 0,0,0 });
		}
	}
}

void DrawToolsPanel(WindowData* windowData, int2 bottomLeft,
	int2 singleToolTileSize, int yDistanceToNextToolTile)
{
	for (int i = 0; i < windowData->toolTiles.length; i++)
	{
		ToolTile tool = windowData->toolTiles.get(i);

		// TODO: create some of kind of a registry of ui ids to prevent collisions
		DrawBitmapButton(windowData,
			{ bottomLeft.x, bottomLeft.y + (i * singleToolTileSize.y) + (i * yDistanceToNextToolTile) },
			tool.image.size, tool.image.rgbaBitmap, { 20, 80, 200 }, tool.uiElement);

		if (tool.tool == windowData->selectedTool)
		{
			DrawBorderRect(windowData, { bottomLeft.x - 1, bottomLeft.y + (i * singleToolTileSize.y) + (i * yDistanceToNextToolTile) - 1 },
				{ tool.image.size.x + 1, tool.image.size.y + 1 }, 2, { 0,0,0 });
		}
	}
}

void DrawDraggableCornerOfDrawingZone(WindowData* windowData)
{
	DrawButton(windowData,
		windowData->drawingZoneCornerResize.xy(),
		windowData->drawingZoneCornerResize.size(), { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::CANVAS_CORNER_RESIZE);
}

void DrawTextBlock(WindowData* windowData)
{
	if (!windowData->isTextEnteringMode)
	{
		return;
	}

	CheckHotActiveForUiElement(windowData, windowData->textBlockOnClient, UI_ELEMENT::TEXT_BLOCK);
	
	DrawBorderRect(windowData,
		windowData->textBlockOnClient.xy(),
		windowData->textBlockOnClient.size(), 1, { 0,255,0 });

	int2 textSelectionRange = { -1, -1 };
	if (windowData->selectedTextStartIndex != -1)
	{
		textSelectionRange = GetSelectedTextRange(windowData);
	}

	// render text
	int textBlockleftSide = windowData->textBlockOnClient.z;
	int maxLinesInTextBlock = windowData->textBlockOnClient.size().y / windowData->fontData.lineHeight;
	int topLineIndex = windowData->topLineIndexToShow;
	for (int layoutLineIndex = topLineIndex, lineIndex = 0; layoutLineIndex < windowData->glyphsLayout->length; layoutLineIndex++, lineIndex++)
	{
		if (lineIndex >= maxLinesInTextBlock)
		{
			break;
		}

		auto line = windowData->glyphsLayout->get(layoutLineIndex);
		int lineTopOffset = windowData->textBlockOnClient.w - (lineIndex + 1) * windowData->fontData.lineHeight;
		for (int j = 0; j < line.length; j++)
		{
			int2 glyphData = line.get(j);
			int charIndex = glyphData.y;

			// check if text in selection if any
			bool isSymbolInSelection = (charIndex >= textSelectionRange.x && charIndex < textSelectionRange.y);

			wchar_t code = windowData->textBuffer.chars[charIndex];
			int lineLeftOffset = windowData->textBlockOnClient.x + glyphData.x;

			if (code != L'\n' && code != L'\0') // glyphs layout includes \0 at the end of the last line
			{
				RasterizedGlyph rasterizedGlyph = windowData->fontData.glyphs.get(code);
				int2 position = {
					lineLeftOffset + rasterizedGlyph.leftSideBearings * windowData->drawingZoomLevel,
					lineTopOffset + (rasterizedGlyph.boundaries.y + -windowData->fontData.descent) * windowData->drawingZoomLevel };

				if (isSymbolInSelection)
				{
					DrawRect(windowData, lineLeftOffset, lineTopOffset,
						rasterizedGlyph.advanceWidth,
						windowData->fontData.lineHeight, { 0,0,0 });
				}

				if (rasterizedGlyph.hasBitmap)
				{
					CopyMonochromicBitmapToBitmap(rasterizedGlyph.bitmap, rasterizedGlyph.bitmapSize,
						windowData->windowBitmap, position, windowData->windowClientSize,
						windowData->drawingZoomLevel, isSymbolInSelection);
				}
			}

			// NOTE: if new line symbol in the selection, we have to highlight it manually
			if (code == L'\n' && isSymbolInSelection)
			{
				int newLineSymbolSelectionWidth = 20;

				if (lineLeftOffset + newLineSymbolSelectionWidth > textBlockleftSide)
				{
					newLineSymbolSelectionWidth = textBlockleftSide - lineLeftOffset;
				}

				DrawRect(windowData, lineLeftOffset, lineTopOffset,
					newLineSymbolSelectionWidth,
					windowData->fontData.lineHeight, { 0, 0, 0 });
			}

			if (charIndex == windowData->cursorPosition)
			{
				DrawRect(windowData, lineLeftOffset, lineTopOffset, 1, windowData->fontData.lineHeight, { 0,0,0 });
			}
		}
	}
}

void DrawTextBlockResizeButtons(WindowData* windowData)
{
	if (!windowData->isTextEnteringMode) return;

	//> corner resize buttons
	DrawButton(windowData,
		{ windowData->textBlockOnClient.x - windowData->textBlockButtonsSize.x, windowData->textBlockOnClient.w },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_TOP_LEFT_CORNER_RESIZE);

	DrawButton(windowData,
		{ windowData->textBlockOnClient.z, windowData->textBlockOnClient.w },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_TOP_RIGHT_CORNER_RESIZE);

	DrawButton(windowData,
		{ windowData->textBlockOnClient.z, windowData->textBlockOnClient.y - windowData->textBlockButtonsSize.y },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_BOTTOM_RIGHT_CORNER_RESIZE);

	DrawButton(windowData,
		{ windowData->textBlockOnClient.x - windowData->textBlockButtonsSize.x, windowData->textBlockOnClient.y - windowData->textBlockButtonsSize.y },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_BOTTOM_LEFT_CORNER_RESIZE);
	//<

	//> edges resize buttons
	DrawButton(windowData,
		{ 
			windowData->textBlockOnClient.x + (windowData->textBlockOnClient.size().x - windowData->textBlockButtonsSize.x) / 2,
			windowData->textBlockOnClient.w
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_TOP_RESIZE);

	DrawButton(windowData,
		{
			windowData->textBlockOnClient.z,
			windowData->textBlockOnClient.y + (windowData->textBlockOnClient.size().y - windowData->textBlockButtonsSize.y) / 2
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_RIGHT_RESIZE);

	DrawButton(windowData,
		{ 
			windowData->textBlockOnClient.x - windowData->textBlockButtonsSize.x, 
			windowData->textBlockOnClient.y + (windowData->textBlockOnClient.size().y - windowData->textBlockButtonsSize.y) / 2
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_LEFT_RESIZE);

	DrawButton(windowData,
		{
			windowData->textBlockOnClient.x + (windowData->textBlockOnClient.size().x - windowData->textBlockButtonsSize.x) / 2,
			windowData->textBlockOnClient.y - windowData->textBlockButtonsSize.y
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_BOTTOM_RESIZE);
	//<
}

void CheckHotActiveForUiElement(WindowData* windowData, int4 boundaries, UI_ELEMENT uiElement)
{
	bool isInRect = IsInRect(boundaries, windowData->mousePosition);

	if (isInRect)
	{
		windowData->hotUi = uiElement;
	}
	//windowData->hotUi = isInRect ? uiElement : UI_ELEMENT::NONE;

	if (windowData->activeUi == uiElement)
	{
		if (windowData->wasRightButtonReleased)
		{
			if (windowData->hotUi == uiElement) windowData->sumbitedUi = uiElement;

			windowData->sumbitedOnAnyHotUi = uiElement;
			windowData->activeUi = UI_ELEMENT::NONE;
			windowData->activeUiOffset = { -1, -1 };
		}
	}
	else if (windowData->hotUi == uiElement)
	{
		if (windowData->wasRightButtonPressed)
		{
			windowData->activeUiOffset = { windowData->mousePosition.x - boundaries.x, windowData->mousePosition.y - boundaries.y };
			windowData->activeUi = uiElement;
		}
	}
}
