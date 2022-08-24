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
		void SetGeometry();
	};

	/**
	 * \brief Data required to define a camera
	 */
	struct CameraDescription
	{


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
		inline std::vector<Object>& GetObjects() { return objects; }
		inline const std::vector<Object>& GetObjectsConst() { return objects; }

		// If should use lighting
		bool enableLight = true;

		// Camera specification
		CameraDescription camera;

		// Output image specification
		float imgHeight, imgWidth; // Height Possibly unused
		size_t imgResX, imgResY;
		float imgDistanceToViewplane; // Possibly unused

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

		Camera(glm::vec3 up = { 0,0,0 }, glm::vec3 position = { 0,0,0 }, glm::vec3 posToLookAt = {0,0,0});

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
		 * \param distanceToViewPlane Distance from the camera (or eye) to the view plane, also called focal length
		 */
		RayGenerator(Camera camera = Camera(), float width = 0, float height = 0, size_t resolutionX = 0, size_t resolutionY = 0, float distanceToViewPlane = 0)
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

		const Camera& GetCamera() const { return m_Camera; }

	private:
		Camera m_Camera;
		// amount of pixels in each axis
		size_t m_PixelsX, m_PixelsY;

		// Where to start each side of the viewing plane, using the coordinate frame from the camera
		float m_Left, m_Right, m_Bottom, m_Top;

		// Distance from camera to viewing plane
		float m_DistanceToViewPlane;
	};

	struct RayIntersectionResult
	{
		std::shared_ptr<const Object*> object;
		glm::vec3 normal;
		glm::vec3 position; // world coordinates
		float t; // Intersection point in ray. How far from origin 

		bool WasIntersection() const { return object != nullptr; }
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

	private:
		/**
		 * \brief Utility function to compute focal length from camera configuration
		 * \param fovy Fovy for camera specification
		 * \param height Height of view plane
		 * \return focal length, distance from eye to viewplane
		 */
		static float FocalLength(float fovy, float height);

		/**
		 * \brief Utility function to compute height of image based on width and aspect ratio
		 * \param aspectRatio Aspect ratio (width / height) from image
		 * \param width Widht of image
		 * \return Expected height
		 */
		static float HeightFromAspectRatio(float aspectRatio, float width);

		/**
		 * \brief Set up geometry of objects in scene description so it matches with the camera
		 */
		void SetUpGeometry();

		/**
		 * \brief Intersect a ray and return a description of the intersection point, if any.
		 * Returned object is guaranteed to be the nearest
		 * \param ray Intersect the provided ray with some objects in the scene
		 * \return Result describing intersection point if any
		 */
		RayIntersectionResult IntersectRay(const Ray& ray);

		/**
		 * \brief Perform ray intersection between the provided ray and object
		 * \param ray Ray to intersect
		 * \param object Object to check for intersection, with vertices in world coordinates
		 * \param minT = Minimum T value for ray
		 * \param maxT = Maximum T value for ray. You can replace this if you already found an object and you're
		 * not interested in any object further away
		 * \return Ray intersection description if any
		 */
		RayIntersectionResult IntersectRayToObject(const Ray& ray, const Object& object, float minT = 0, float maxT = INFINITY) const;

		/**
		 * \brief Check for intersection between the provided ray and sphere. Will crash if
		 * object is not of type sphere
		 * \param ray Ray to intersect
		 * \param sphere Object of type sphere to check for intersection
		 * \return Intersection description
		 */
		RayIntersectionResult IntersectRayToSphere(const Ray& ray, const Object& sphere, float minT = 0, float maxT = INFINITY) const;

		/**
		 * \brief Intersect a ray to the triangle specified by the given vertices. Winding order does not
		 * matter.
		 * \param ray Ray to intersect
		 * \param v1 First vertex of triangle 
		 * \param v2 Second vertex of triangle 
		 * \param v3 Third vertex of triangle
		 * \param n1 normal fist vertex of triangle 
		 * \param n2 normal second vertex of triangle 
		 * \param n3 normal third vertex of triangle
		 * \param outIntersection position of intersection with given triangle
		 * \param outNormal interpolated normal at intersection
		 * \param outT where in the ray the intersection happened 
		 * \param minT minimum acceptable T (point from eye position)
		 * \param maxT minimum acceptable T (point from eye position)
		 * \return If there was an intersection
		 */
		static bool IntersectRayToTriangle(
			const Ray& ray, 
			const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, 
			const glm::vec3& n1, const glm::vec3& n2, const glm::vec3& n3, 
			glm::vec3& outIntersection, glm::vec3& outNormal, float& outT,
			float minT, float maxT);

		RayIntersectionResult IntersectRayToTesselatedObject(const Ray& ray, const Object& object, float minT = 0, float maxT = INFINITY) const;

		/**
		 * \brief select color using global information and ray intersection information
		 * \param rayIntersection Compute color of corresponding pixel from the global information and
		 *			ray intersection information
		 * \return color corresponding to this pixel
		 */
		glm::vec4 Shade(const RayIntersectionResult& rayIntersection);
	};

	/**
	 * \brief Simple class to draw a progress bar
	 */
	class ProgressBar
	{
	public:
		ProgressBar(size_t nSteps, size_t barSize = 70)
			: m_BarSize(barSize)
			, m_NSteps(nSteps)
			, m_CurrentSteps(0)
		{ }

		void Step();

		void Draw() const;

	private:
		size_t m_BarSize;
		size_t m_NSteps;
		size_t m_CurrentSteps;

	};
}
