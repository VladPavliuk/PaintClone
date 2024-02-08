#include "math.h"

bool IsInRect(int4 rect, int2 point)
{
	return point.x > rect.x && point.x < rect.z
		&& point.y > rect.y && point.y < rect.w;
}
