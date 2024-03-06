#pragma once

#include "custom_types.h"
#include "dynamic_array.h"

bool IsInRect(int4 rect, int2 point);

int GenerateBezierCurve(int2 p0, int2 p1, int2 p2, int pointsToInterpolate, SimpleDynamicArray<int2>* points);

int absInt(int a);

int maxInt(int a, int b);

int minInt(int a, int b);

float absFloat(float a);

float maxFloat(float a, float b);

float minFloat(float a, float b);

int4 ClipRect(int4 rectSource, int4 rectDest);
int4 ClipRect(int4 rectSource, int2 rectDest);
int2 ClipPoint(int2 point, int4 rect);
int ClipPoint(int point, int2 range);
