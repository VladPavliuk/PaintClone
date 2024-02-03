#pragma once

#include "window_data.h"

void InitRenderer(WindowData* windowData, HWND hwnd);

void DrawLine(WindowData* windowData, int2 from, int2 to, ubyte3 color);
void FillWindowClientWithWhite(WindowData* windowData);

void FillFromPixel(WindowData* windowData, int2 fromPixel, ubyte3 color);

void DrawRect(WindowData* windowData, int x, int y, int width, int height, ubyte3 color);
void DrawBorderRect(WindowData* windowData, int2 bottomLeft, int2 size, int lineWidth, ubyte3 color);

inline ubyte3 GetPixelColor(WindowData* windowData, int x, int y);
inline void DrawPixel(WindowData* windowData, int x, int y, ubyte3 color);
