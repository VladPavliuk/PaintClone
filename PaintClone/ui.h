#pragma once

#include "renderer.h"
#include "math.h"

//void DrawRectToZAndIdBuffer(WindowData* windowData,
//	int2 bottomLeft,
//	int2 size,
//	ubyte zIndex,
//	ubyte uiId);

//ubyte2 GetZAndIdFromBuffer(WindowData* windowData, int2 position);

void HandleUiElements(WindowData* windowData);

void DrawDrawingCanvas(WindowData* windowData);

void DrawScrollsForDrawingZone(WindowData* windowData);

void DrawPanel(
	WindowData* windowData,
	int2 bottomLeft,
	int2 size,
	ubyte3 bgColor,
	ubyte zIndex,
	ubyte uiId);

void DrawBitmapButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte4* bitmap,
	ubyte3 hoveredBgColor,
	UI_ELEMENT uiElement);

void DrawButton(
	WindowData* windowData,
	int2 bottomLeft,
	int2 size,
	ubyte3 bgColor,
	ubyte3 hoveredBgColor,
	UI_ELEMENT uiElement);

void DrawColorsBrush(WindowData* windowData, SimpleDynamicArray<BrushColorTile>* colors, int2 bottomLeft,
	int2 singleColorTileSize, int xDistanceToNextColor);

void DrawToolsPanel(WindowData* windowData, int2 bottomLeft,
	int2 singleToolTileSize, int yDistanceToNextToolTile);

void CheckHotActiveForUiElement(WindowData* windowData, int4 boundaries, UI_ELEMENT uiElement);