#include "ui.h"

void DrawRectToZAndIdBuffer(WindowData* windowData,
	int2 bottomLeft,
	int2 size,
	ubyte zIndex,
	ubyte uiId)
{
	ubyte2* currentElement = windowData->zAndIdBuffer + (bottomLeft.x + windowData->clientSize.x * bottomLeft.y);
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
	if (position.x < 0 || position.y < 0 || position.x >= windowData->clientSize.x || position.y >= windowData->clientSize.y)
	{
		return { 0, 0 };
	}

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

bool DrawBitmapButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte4* bitmap,
	ubyte3 hoveredBgColor,
	ubyte zIndex, ubyte uiId)
{
	ubyte2 zAndId = GetZAndIdFromBuffer(windowData, windowData->mousePosition);

	if (zAndId.y == uiId)
	{
		// TODO: we will able to see some background after we move to RGBA bitmap
		DrawRect(windowData, bottomLeft.x, bottomLeft.y, size.x, size.y, hoveredBgColor);
	}

	//TODO: add some kind of image stretching because we might want to have buttons that dont have the same size as a bitmap
	DrawBitmap(windowData, bitmap, bottomLeft, size);

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
	DRAW_TOOL tools[] = {
		DRAW_TOOL::PENCIL,
		DRAW_TOOL::FILL
	};

	for (int i = 0; i < ARRAYSIZE(tools); i++)
	{
		DRAW_TOOL tool = tools[i];
		BmpImage toolImage = windowData->toolsImages.get((int)tool);

		// TODO: create some of kind of a registry of ui ids to prevent collisions
		if (DrawBitmapButton(windowData,
			{ bottomLeft.x, bottomLeft.y + (i * singleToolTileSize.y) + (i * yDistanceToNextToolTile) },
			toolImage.size, toolImage.rgbaBitmap, { 20, 80, 200 },
			1, 10 + i))
		{
			windowData->selectedTool = tool;
		}
	}
}
