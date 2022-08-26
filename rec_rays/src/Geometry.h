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
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
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
		static void LoadTeapotGeometry();
		static void LoadCubeGeometry();
	private:
		inline static Geometry s_CubeGeometry;
		inline static Geometry s_TeapotGeometry;
		inline static bool s_Initialized = false;
		static constexpr char* s_PathToTeapotObj = "models/teapot.obj";
		static constexpr char* s_PathToCubeObj = "models/cube.obj";
	};
}
