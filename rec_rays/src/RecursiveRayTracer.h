#pragma once
// STL Includes
#include <vector>
#include <memory>

// Third party includes
#include <glm/glm.hpp>
#include <FreeImage.h>

// Local includes
#include "Geometry.h"

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
		Geometry geometry; // empty geometry when it's sphere

		/**
		 * \brief Set up geometry ptr according to the shape
		 */
		void SetGeometry(const glm::mat4& ViewMatrix);
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

		// If should use lighting
		bool enableLight = true;

		// Camera specification
		CameraDescription camera;

		// Image specification
		float imgWidth, imgHeight;	// Dimensions of output image
		float imgDistanceToViewplane; // Distance from viewpoint to viewplane
		size_t imgResX, imgResY;	// Resolution in each axis for this image


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

		/**
		 * \brief Compute a model view matrix as a look at matrix
		 * \return A model view matrix implemented as a look at matrix
		 */
		glm::mat4 GetModelViewMatrix() const;

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

	struct Ray
	{
		glm::vec3 position;
		glm::vec3 direction;
	};

	class RayGenerator
	{
	public:
		/**
		 * \brief Specifies possible variants for the ray generation process,
		 * MID is in the middle of the pixel, TOP_LEFT is in the top left corner, and RANDOMIZED is mid with a small
		 * variation
		 */
		enum class RayType
		{
			MID,
			TOP_LEFT,
			RANDOMIZED
		};
	public:
		RayGenerator(Camera camera, size_t pixelsX, size_t pixelsY, float left, float right, float bottom, float top, float distanceToViewPlane)
			: m_Camera(camera)
			, m_PixelsX(pixelsX)
			, m_PixelsY(pixelsY)
			, m_Left(left)
			, m_Right(right)
			, m_Bottom(bottom)
			, m_Top(top)
			, m_DistanceToViewPlane(distanceToViewPlane)
		{ }

		/**
		 * \brief Create a ray generator from an image specification
		 * \param camera Camera with valid coordinate axis
		 * \param width width of expected image
		 * \param height height of expected image
		 * \param resolutionX How many pixels in X
		 * \param resolutionY How many pixels in Y
		 */
		RayGenerator(Camera camera, float width, float height, size_t resolutionX, size_t resolutionY, float distanceToViewPlane)
			: RayGenerator(camera, resolutionX, resolutionY, width / 2.f, width / 2.f, height / 2.f, height / 2.f, distanceToViewPlane)
		{ }

		/**
		 * \brief Generate a ray that will pass through the specified pixel according to its X and Y indices
		 * \param pixelX Index in X axis for this pixel, starting from left to right
		 * \param pixelY Index in Y axis for this pixel, starting from top to bottom
		 * \param type Type of ray to generate
		 * \return Resulting ray	
		 */
		Ray GetRayThroughPixel(size_t pixelX, size_t pixelY, RayType type = RayType::MID);

	private:
		Camera m_Camera;
		// amount of pixels in each axis
		size_t m_PixelsX, m_PixelsY;

		// Where to start each side of the viewing plane, using the coordinate frame from the camera
		float m_Left, m_Right, m_Bottom, m_Top;

		// Distance from camera to viewing plane
		float m_DistanceToViewPlane;
	};

	class RecursiveRayTracer
	{
	public:
		RecursiveRayTracer(const SceneDescription& description);

		int Draw(FIBITMAP*& outImage);

	private:
		// Scene to render 
		SceneDescription m_SceneDescription;
		// Object to generate rays from scene description
		RayGenerator m_RayGenerator;

	};
}
