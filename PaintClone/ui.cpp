#include "ui.h"

void DrawRectToZAndIdBuffer(WindowData* windowData,
	int2 topLeft,
	int2 size,
	ubyte zIndex,
	ubyte uiId)
{
	ubyte2* currentElement = windowData->zAndIdBuffer + (topLeft.x + windowData->clientSize.x * topLeft.y);
	int pitch = windowData->clientSize.x;

	for (int i = 0; i < size.y; i++)
	{
		for (int j = 0; j < size.x; j++)
		{
			*currentElement = { zIndex, uiId };

			currentElement++;
		}

		currentElement -= size.x;
		currentElement += pitch;
	}
}

ubyte2 GetZAndIdFromBuffer(WindowData* windowData, int2 position)
{
	return *(windowData->zAndIdBuffer + (position.x + windowData->clientSize.x * position.y));
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

bool DrawButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte3 bgColor, ubyte3 hoveredBgColor,
	ubyte zIndex, ubyte uiId)
{
	ubyte2 zAndId = GetZAndIdFromBuffer(windowData, windowData->mousePosition);

	if (zAndId.y == uiId)
	{
		DrawRect(windowData, bottomLeft.x, bottomLeft.y, size.x, size.y, bgColor);
	}
	else
	{
		DrawRect(windowData, bottomLeft.x, bottomLeft.y, size.x, size.y, hoveredBgColor);
	}

	DrawBorderRect(windowData, bottomLeft, size, 1, { 0,0,0 });

	DrawRectToZAndIdBuffer(windowData,
		bottomLeft, size,
		zIndex, uiId);

	// NOTE: if we in the process of drawing, ignore any action performed by button
	if (windowData->isDrawing)
	{
		return false;
	}

	if (zAndId.y == uiId && windowData->isRightButtonHold)
	{
		return true;
	}

	return false;
}

void DrawColorsBrush(WindowData* windowData, SimpleDynamicArray<ubyte3>* colors, int2 bottomLeft,
	int2 singleColorTileSize, int xDistanceToNextColor)
{
	for (int i = 0; i < colors->length; i++)
	{
		ubyte3 color = colors->get(i);

		if (DrawButton(windowData,
			{ bottomLeft.x + (singleColorTileSize.x * i) + (xDistanceToNextColor * i), bottomLeft.y },
			singleColorTileSize, color, color, 1, i + 1))
		{
			windowData->selectedColor = color;
		}
	}
}

void DrawToolsPanel(WindowData* windowData, int2 bottomLeft,
	int2 singleToolTileSize, int yDistanceToNextToolTile)
{
	if (DrawButton(windowData,
		{ bottomLeft.x, bottomLeft.y },
		singleToolTileSize, { 90,0,90 }, { 120,0,120 }, 1, 11))
	{
		windowData->selectedTool = DRAW_TOOL::PENCIL;
	}

	if (DrawButton(windowData,
		{ bottomLeft.x, bottomLeft.y + 30 },
		singleToolTileSize, { 90,90,0 }, { 120,120,0 }, 1, 12))
	{
		windowData->selectedTool = DRAW_TOOL::FILL;
	}
}
