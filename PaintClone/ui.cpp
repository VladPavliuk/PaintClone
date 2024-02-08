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

			pixelToStart.x -= windowData->drawingZonePosition.x;
			pixelToStart.y -= windowData->drawingZonePosition.y;

			pixelToStart.x += windowData->drawingOffset.x;
			pixelToStart.y += windowData->drawingOffset.y;

			pixelToStart.x = (int)((float)pixelToStart.x / (float)windowData->drawingZoomLevel);
			pixelToStart.y = (int)((float)pixelToStart.y / (float)windowData->drawingZoomLevel);

			FillFromPixel(windowData, pixelToStart, windowData->selectedColor);
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

	switch (windowData->activeUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	case UI_ELEMENT::DRAWING_VERTICAL_SCROLL:
	{
		windowData->drawingOffset.y += windowData->drawingZoomLevel * (windowData->mousePosition.y - windowData->prevMousePosition.y);

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::DRAWING_HORIZONTAL_SCROLL:
	{
		windowData->drawingOffset.x += windowData->drawingZoomLevel * (windowData->mousePosition.x - windowData->prevMousePosition.x);

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		if (windowData->hotUi == UI_ELEMENT::DRAWING_CANVAS)
		{
			if (windowData->prevHotUi != UI_ELEMENT::DRAWING_CANVAS)
			{
				break;
			}

			if (windowData->selectedTool == DRAW_TOOL::PENCIL)
			{
				int2 fromPixel = windowData->prevMousePosition;
				int2 toPixel = windowData->mousePosition;

				fromPixel.x -= windowData->drawingZonePosition.x;
				fromPixel.y -= windowData->drawingZonePosition.y;

				toPixel.x -= windowData->drawingZonePosition.x;
				toPixel.y -= windowData->drawingZonePosition.y;

				fromPixel.x += windowData->drawingOffset.x;
				fromPixel.y += windowData->drawingOffset.y;

				toPixel.x += windowData->drawingOffset.x;
				toPixel.y += windowData->drawingOffset.y;

				fromPixel.x = (int)((float)fromPixel.x / (float)windowData->drawingZoomLevel);
				fromPixel.y = (int)((float)fromPixel.y / (float)windowData->drawingZoomLevel);

				toPixel.x = (int)((float)toPixel.x / (float)windowData->drawingZoomLevel);
				toPixel.y = (int)((float)toPixel.y / (float)windowData->drawingZoomLevel);

				if (fromPixel.x == toPixel.x && fromPixel.y == toPixel.y)
				{
					DrawPixel(windowData->drawingBitmap, windowData->drawingBitmapSize, toPixel, windowData->selectedColor);
				}
				else
				{
					DrawLine(windowData->drawingBitmap, windowData->drawingBitmapSize, fromPixel, toPixel, windowData->selectedColor);
				}
			}
		}
		else
		{
			//windowData->pixelsToDraw.clear();
		}
		break;
	}
	}
}

void DrawDrawingCanvas(WindowData* windowData)
{
	DrawBorderRect(windowData, { windowData->drawingZonePosition.x, windowData->drawingZonePosition.y },
		windowData->drawingZoneSize, 2, { 255, 0, 0 });
	DrawScrollsForDrawingZone(windowData);

	CheckHotActiveForUiElement(windowData, {
		windowData->drawingZonePosition.x, windowData->drawingZonePosition.y,
		windowData->drawingZonePosition.x + windowData->drawingZoneSize.x, windowData->drawingZonePosition.y + windowData->drawingZoneSize.y
		}, UI_ELEMENT::DRAWING_CANVAS);
}

void DrawScrollsForDrawingZone(WindowData* windowData)
{
	int scrollWidth = 10;
	ubyte3 scrollBgColor = { 150, 150, 150 };
	ubyte3 scrollHoveredBgColor = { 120, 120, 120 };

	// vertical bar
	float drawingZoneToImageHeightRatio = (float)windowData->drawingZoneSize.y
		/ ((float)windowData->drawingBitmapSize.y * (float)windowData->drawingZoomLevel);

	if (drawingZoneToImageHeightRatio < 1.0f)
	{
		DrawButton(windowData,
			{
				windowData->drawingZonePosition.x - scrollWidth,
				(int)(drawingZoneToImageHeightRatio * windowData->drawingOffset.y) + windowData->drawingZonePosition.y
			},
			{ scrollWidth, (int)(drawingZoneToImageHeightRatio * windowData->drawingZoneSize.y) },
			scrollBgColor, scrollHoveredBgColor, UI_ELEMENT::DRAWING_VERTICAL_SCROLL);
	}

	// horizontal bar
	float drawingZoneToImageWidthRatio = (float)windowData->drawingZoneSize.x / ((float)windowData->drawingBitmapSize.x * (float)windowData->drawingZoomLevel);

	if (drawingZoneToImageWidthRatio < 1.0f)
	{
		DrawButton(windowData,
			{
				(int)(drawingZoneToImageWidthRatio * windowData->drawingOffset.x) + windowData->drawingZonePosition.x,
				windowData->drawingZonePosition.y - scrollWidth,
			},
			{ (int)(drawingZoneToImageWidthRatio * windowData->drawingZoneSize.x), scrollWidth },
			scrollBgColor, scrollHoveredBgColor, UI_ELEMENT::DRAWING_HORIZONTAL_SCROLL);
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

void CheckHotActiveForUiElement(WindowData* windowData, int4 boundaries, UI_ELEMENT uiElement)
{
	bool isInRect = IsInRect(boundaries, windowData->mousePosition);

	windowData->hotUi = isInRect ? uiElement : UI_ELEMENT::NONE;

	if (windowData->activeUi == uiElement)
	{
		if (windowData->wasRightButtonReleased)
		{
			if (windowData->hotUi == uiElement) windowData->sumbitedUi = uiElement;

			windowData->activeUi = UI_ELEMENT::NONE;
		}
	}
	else if (windowData->hotUi == uiElement)
	{
		if (windowData->wasRightButtonPressed) windowData->activeUi = uiElement;
	}
}
