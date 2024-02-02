#pragma once

#include "window_data.h"

void InitRenderer(WindowData* windowData, HWND hwnd);

void FillWindowClientWithWhite(WindowData* windowData);

void DrawRect(WindowData* windowData, int x, int y, int width, int height, char3 color);
