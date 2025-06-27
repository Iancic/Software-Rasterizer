#pragma once
#include "Math.hpp"

class Ray
{
public:
	Ray(const float3 origin, const float3 direction, const float distance = 1e34f)
	{
		O = origin;
		D = direction;
		T = distance;
	};

	float T;
	float3 O, D;

	float3 IntersectionPoint() const { return O + D * T; };
};