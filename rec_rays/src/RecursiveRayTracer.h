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
		glm::ivec2 size; // a scaling factor
		glm::mat4 transform;
	};

	/**
	 * \brief Data required to define a camera
	 */
	struct CameraDescription
	{
		CameraDescription(glm::vec3 _position = { 0,0,0 }, glm::vec3 _lookAt = { 0,0,0 }, glm::vec3 _up = { 0,0,0 }, float _fovy = 0)
			: position(_position)
			, lookAt(_lookAt)
			, up(_up)
			, fovy(_fovy)
		{ }

		glm::vec3 position;	// Camera position in world coordinates
		glm::vec3 lookAt;	// Point where the camera is looking at
		glm::vec3 up;		// up direction
		float fovy;
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

		CameraDescription camera;

	private:
		std::vector<Light> lights;
		std::vector<Object> objects;
	};

	/**
	 * \brief Describes a camera and manages its attributes
	 */
	class Camera
	{
	public:

		Camera(glm::vec3 up, glm::vec3 position, glm::vec3 posToLookAt);

		inline glm::vec3 GetU() const { return m_U; }
		inline glm::vec3 GetV() const { return m_V; }
		inline glm::vec3 GetW() const { return m_W; }

		inline glm::vec3 GetUp() const { return m_Up; }
		inline glm::vec3 GetPosition() const { return m_Position; }
		inline glm::vec3 GetPosToLookAt() const { return m_PosToLookAt; }

		void SetUp(glm::vec3 newUp);
		void SetPosition(glm::vec3 newPosition);
		void SetPosToLookAt(glm::vec3 newPosToLookAt);

	private:
		/**
		 * \brief Update internal state of coordinate axis
		 */
		void UpdateCoordinateAxis();
	private:
		// Configuration parameters
		glm::vec3 m_Up, m_Position, m_PosToLookAt;

		// Coordinate axis
		glm::vec3 m_U, m_V, m_W; 
	};

	class RecursiveRayTracer
	{
	public:
		RecursiveRayTracer(const SceneDescription& description) : m_SceneDescription(description) {}

	private:
		SceneDescription m_SceneDescription;
	};
}
