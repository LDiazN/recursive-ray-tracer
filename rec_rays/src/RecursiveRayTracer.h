#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace RecRays
{
	constexpr int MAX_LIGHTS = 20;
	constexpr int MAX_OBJECTS = 20;

	/**
	 * \brief Light properties
	 */
	struct Light
	{
		glm::vec4 position, color;
	};

	/**
	 * \brief Possible shape variants
	 */
	enum class Shape
	{
		Cube,
		Sphere,
		Teapot
	};

	/**
	 * \brief A scene object's properties
	 */
	struct Object
	{
		// Coloring
		glm::vec4 ambient, diffuse, specular, emission;
		float shininess;

		// Geometry
		Shape shape;
		float size; // a scaling factor
		glm::mat4 transform;
	};

	/**
	 * \brief Data conforming a single scene
	 */
	struct SceneDescription
	{

		bool enableLight = true;

		inline size_t GetNumLights() const { return lights.size(); }
		inline size_t GetNumObjects() const { return objects.size(); }

		/**
		 * \brief Add a new light to light array
		 * \param newLigth Add an object to the ligth vector 
		 * \return Success status, might fail if already max lights
		 */
		int AddLight(const Light& newLigth);
		int AddObject(const Object& newObject);

		inline const std::vector<Light>& GetLights() { return lights; }
		inline const std::vector<Object>& GetObjects() { return objects; }

	private:
		std::vector<Light> lights;
		std::vector<Object> objects;
	};

	class RecursiveRayTracer
	{
	public:
		RecursiveRayTracer(const SceneDescription& description) : m_SceneDescription(description) {}

	private:
		SceneDescription m_SceneDescription;
	};
}
