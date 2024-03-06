#pragma once

#include <cmath>

#include "renderer.h"
#include "math.h"
#include "text.h"
#include "dialog_window.h"

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

void DrawModalWindow(WindowData* windowData, WideString* title, int4 modalWindowBox, UI_ELEMENT uiElement);

void DrawColorsBrush(WindowData* windowData, SimpleDynamicArray<BrushColorTile>* colors, int2 bottomLeft,
	int2 singleColorTileSize, int xDistanceToNextColor);

void DrawToolsPanel(WindowData* windowData, int2 bottomLeft,
	int2 singleToolTileSize, int yDistanceToNextToolTile);

void DrawCanvasSizeLabel(WindowData* windowData);

void DrawMouseCanvasPositionLabel(WindowData* windowData);

void DrawDraggableCornerOfDrawingZone(WindowData* windowData);

void DrawTextBlock(WindowData* windowData);
void DrawTextBlockResizeButtons(WindowData* windowData);

void CheckHotActiveForUiElement(WindowData* windowData, int4 boundaries, UI_ELEMENT uiElement);
