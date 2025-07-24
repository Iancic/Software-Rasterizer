#pragma once
#include "Math.hpp"

class PointLight
{
	public:
		PointLight();
		~PointLight();

		float3 position;
		float3 intensity;

	private:
};