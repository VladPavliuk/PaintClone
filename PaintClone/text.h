#pragma once

#include "renderer.h"

void DrawTextLine(WideString* string, int2 bottomLeft, FontDataRasterized* font, ubyte4* bitmap, int2 bitmapSize);

void UpdateTextBlockTopLine(WindowData* windowData);

void UpdateTextSelectionifShiftPressed(WindowData* windowData);

void MoveCursorToNewLine(WindowData* windowData, int newLineIndex, int oldCursorLeftOffset);
int2 GetSelectedTextRange(WindowData* windowData);

int2 GetCursorLayoutPotision(WindowData* windowData);
int GetCursorPositionByMousePosition(WindowData* windowData);

void RecreateGlyphsLayout(WindowData* windowData, WideString text, int lineMaxWidth);