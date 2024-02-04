#pragma once

#include "renderer.h"

void DrawRectToZAndIdBuffer(WindowData* windowData,
	int2 bottomLeft,
	int2 size,
	ubyte zIndex,
	ubyte uiId);

ubyte2 GetZAndIdFromBuffer(WindowData* windowData, int2 position);

void DrawPanel(
	WindowData* windowData,
	int2 bottomLeft,
	int2 size,
	ubyte3 bgColor,
	ubyte zIndex,
	ubyte uiId);

bool DrawBitmapButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte4* bitmap,
	ubyte3 hoveredBgColor,
	ubyte zIndex, ubyte uiId);

bool DrawButton(
	WindowData* windowData,
	int2 bottomLeft,
	int2 size,
	ubyte3 bgColor,
	ubyte3 hoveredBgColor,
	ubyte zIndex,
	ubyte uiId);

void DrawColorsBrush(WindowData* windowData, SimpleDynamicArray<ubyte3>* colors, int2 bottomLeft,
	int2 singleColorTileSize, int xDistanceToNextColor);

void DrawToolsPanel(WindowData* windowData, int2 bottomLeft,
	int2 singleToolTileSize, int yDistanceToNextToolTile);