#pragma once

namespace RecRays
{
	struct SceneDescription
	{
			
	};

	class RecursiveRayTracer
	{
	public:
		RecursiveRayTracer(const SceneDescription& description) : m_SceneDescription(description) {}

	private:
		SceneDescription m_SceneDescription;
	};
}
