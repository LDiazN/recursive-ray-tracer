// Code required to load geometry for multiple shapes
#pragma once

// STL includes
#include <vector>

// Third party includes
#include <glm/glm.hpp>

namespace RecRays
{
	struct Geometry
	{
		std::vector<glm::vec3> geometry;
		std::vector<glm::uvec3> indices;
	};

	/**
	 * \brief Load geometry from the start for common shapes and query it whenever it's necessary
	 */
	class GeometryLoader
	{
	public:
		/**
		 * \brief Initialize static class. Load geometry for available objects and store it in static mem
		 */
		static void Init();

		/**
		 * \brief Destroys stored geometry
		 */
		static void Shutdown();

		static Geometry GetCubeGeometry();
		static Geometry GetTeapotGeometry();

	private:
		void LoadTeapotGeometry();
		void LoadCubeGeometry();
	private:
		static Geometry s_CubeGeometry;
		static Geometry s_TeapotGeometry;
		inline static bool s_Initialized = false;
	};
}
