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
		float t1 = (1.0 - t);
		float t2 = t * t;
		float x = t1 * t1 * p0.x + 2 * t1 * t * p1.x + t2 * p2.x;
		float y = t1 * t1 * p0.y + 2 * t1 * t * p1.y + t2 * p2.y;

		points->add({ (int)x, (int)y });
	}

	return pointsAdded;
}
