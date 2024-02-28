#include "ui.h"

void HandleUiElements(WindowData* windowData)
{
	switch (windowData->sumbitedUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
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
			//int2 locationOnCanvas = windowData->mousePosition;

			//locationOnCanvas = ConvertFromScreenToDrawingCoords(windowData, locationOnCanvas);

			if (!IsInRect(windowData->drawingZone, windowData->mousePosition))
			{
				break;
			}

			if (windowData->isTextEnteringMode)
			{
				break;
			}

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

			windowData->isTextEnteringMode = true;
			windowData->textBlockOnClient = textBlockRect;

			RecreateGlyphsLayout(windowData, windowData->textBuffer, textBlockRect.size().x);

			windowData->cursorPosition = 0;
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
	}
	windowData->sumbitedUi = UI_ELEMENT::NONE;

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
	windowData->sumbitedOnAnyHotUi = UI_ELEMENT::NONE;

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

			if (!IsInRect({ windowData->drawingZone.x, windowData->drawingZone.y, windowData->drawingZone.z, windowData->drawingZone.w }, fromPixel)
				&& !IsInRect({ windowData->drawingZone.x, windowData->drawingZone.y, windowData->drawingZone.z, windowData->drawingZone.w }, toPixel))
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
	}
}

void DrawDrawingCanvas(WindowData* windowData)
{
	DrawBorderRect(windowData, { windowData->drawingZone.x, windowData->drawingZone.y },
		windowData->drawingZone.size(), 2, {255, 0, 0});
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
			{ scrollWidth, (int)(drawingZoneToImageHeightRatio * windowData->drawingZone.size().y)},
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
			{ (int)(drawingZoneToImageWidthRatio * windowData->drawingZone.size().x), scrollWidth},
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
		{ windowData->drawingZoneCornerResize.x, windowData->drawingZoneCornerResize.y },
		windowData->drawingZoneCornerResize.size(), {100, 100, 100}, {150, 150, 150}, UI_ELEMENT::CANVAS_CORNER_RESIZE);
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
