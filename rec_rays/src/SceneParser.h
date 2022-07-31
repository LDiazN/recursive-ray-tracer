#pragma once
#include "RecursiveRayTracer.h"
#include <string>

namespace RecRays
{
	class SceneParser
	{
		SceneParser();

		int Parse(const std::string& filepath, SceneDescription& outDescription);
	};
}
