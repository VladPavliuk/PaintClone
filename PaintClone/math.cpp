#include "math.h"

bool IsInRect(int4 rect, int2 point)
{
	return point.x >= rect.x && point.x < rect.z
		&& point.y >= rect.y && point.y < rect.w;
}

int GenerateBezierCurve(int2 p0, int2 p1, int2 p2, int pointsToInterpolate, SimpleDynamicArray<int2>* points)
{
	assert(pointsToInterpolate > 0);
	
	float stepPerIter = 1.0f / pointsToInterpolate;
	int pointsAdded = 0;
	for (int i = 0; i <= pointsToInterpolate; i++) {
		pointsAdded++;
		float t = i * stepPerIter;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p0.x + 2 * t1 * t * p1.x + t2 * p2.x;
		float y = t1 * t1 * p0.y + 2 * t1 * t * p1.y + t2 * p2.y;

		points->add({ (int)x, (int)y });
	}

	return pointsAdded;
}

int absInt(int a)
{
	return a >= 0 ? a : -a;
}

int maxInt(int a, int b)
{
	return a > b ? a : b;
}

int minInt(int a, int b)
{
	return a < b ? a : b;
}

float absFloat(float a)
{
	return a >= 0 ? a : -a;
}

float maxFloat(float a, float b)
{
	return a > b ? a : b;
}

float minFloat(float a, float b)
{
	return a < b ? a : b;
}

int4 ClipRect(int4 rectSource, int4 rectDest)
{
	int4 clippedRect = rectSource;

	if (rectSource.x < rectDest.x) clippedRect.x = rectDest.x;
	if (rectSource.y < rectDest.y) clippedRect.y = rectDest.y;

	if (rectSource.z > rectDest.z) clippedRect.z = rectDest.z;
	if (rectSource.w > rectDest.w) clippedRect.w = rectDest.w;

	return clippedRect;
}

int4 ClipRect(int4 rectSource, int2 rectDest)
{
	return ClipRect(rectSource, { 0, 0, rectDest.x, rectDest.y });
}

int2 ClipPoint(int2 point, int4 rect)
{
	int2 clippedPoint = point;

	if (point.x < rect.x) clippedPoint.x = rect.x;
	else if (point.x > rect.z) clippedPoint.x = rect.z;

	if (point.y < rect.y) clippedPoint.y = rect.y;
	else if (point.y > rect.w) clippedPoint.y = rect.w;

	return clippedPoint;
}

int ClipPoint(int point, int2 range)
{
	int clippedPoint = point;

	if (point < range.x) clippedPoint = range.x;
	else if (point > range.y) clippedPoint = range.y;

	return clippedPoint;
}