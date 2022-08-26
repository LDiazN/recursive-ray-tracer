// Local includes
#include "RecursiveRayTracer.h"
#include "RecRays.h"

// STL includes
#include <assert.h>

// Vendor includes
#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>

namespace RecRays
{
	// -- < Scene Description > -------------------------
	int SceneDescription::AddLight(const Light& newLigth)
	{
		// Can't add light if already at max lights
		if (lights.size() == MAX_LIGHTS)
			return FAIL;

		lights.push_back(newLigth);

		return SUCCESS;
	}

	int SceneDescription::AddObject(const Object& newObject)
	{
		// Can't add light if already at max objects
		if (objects.size() == MAX_OBJECTS)
			return FAIL;

		objects.push_back(newObject);

		return SUCCESS;
	}

	// -- < Object > ---------------------------------
	void Object::SetGeometry()
	{
		// Load proper geometry
		switch (shape)
		{
		case Shape::Cube:
			geometry = GeometryLoader::GetCubeGeometry();
			break;
		case Shape::Teapot:
			geometry = GeometryLoader::GetTeapotGeometry();
			break;
		case Shape::Sphere:
			return;
		default:
			assert(false && "Invalid shape");
		}

		// Transform geometry:
		for (auto& vertex : geometry.vertices)
		{
			// Scale object to right size
			vertex = size * vertex;

			// Set up vertices  transformation
			vertex = glm::vec3(transform * glm::vec4(vertex, 1.0f));
		}

		// Set up normals
		auto const inverseTransform = glm::transpose(glm::inverse(transform));
		for(auto& normal : geometry.normals)
		{
			normal = glm::vec3(inverseTransform * glm::vec4(normal, 0.f));
		}
	}

	// -- < Camera > -----------------------------------

	Camera::Camera(glm::vec3 up, glm::vec3 position, glm::vec3 posToLookAt)
		: m_Up(up)
		, m_Position(position)
		, m_PosToLookAt(posToLookAt)
	{
		UpdateCoordinateAxis();
	}

	void Camera::SetUp(glm::vec3 newUp)
	{
		m_Up = newUp;
		UpdateCoordinateAxis();
	}

	void Camera::SetPosition(glm::vec3 newPosition)
	{
		m_Position = newPosition;
		UpdateCoordinateAxis();
	}

	void Camera::SetPosToLookAt(glm::vec3 newPosToLookAt)
	{
		m_PosToLookAt = newPosToLookAt;
		UpdateCoordinateAxis();
	}

	void Camera::UpdateCoordinateAxis()
	{
		auto const v = glm::normalize(m_PosToLookAt - m_Position);
		auto const u = glm::normalize(glm::cross(v, m_Up));
		auto const w = glm::normalize(glm::cross(u, v));

		m_U = u;
		m_V = v;
		m_W = w;
	}

	glm::mat4 Camera::GetModelViewMatrix() const
	{
		glm::mat4 axisTransform = {
			m_U.x, m_U.y, m_U.z, 0.f,
			m_V.x, m_V.y, m_V.z, 0.f,
			m_W.x, m_W.y, m_W.z, 0.f,
			0.f, 0.f, 0.f, 1.f
		};


		glm::mat4 inverseTranslation = {
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			-(m_Position.x - m_PosToLookAt.x), -(m_Position.y - m_PosToLookAt.y), -(m_Position.z - m_PosToLookAt.z), 0.f,
		};

		return axisTransform * inverseTranslation;
	}


	// -- < Ray Generation > ----------------------------------------------

	Ray RayGenerator::GetRayThroughPixel(size_t pixelX, size_t pixelY, RayType type)
	{
		// Find the top left corner to get position of next pixel from this
		glm::vec3 topLeftCorner = -m_Left * m_Camera.GetU() + m_Top * m_Camera.GetW();

		// Use pixel width and height to find how much to offset for each step
		float pixelWidth = (m_Right + m_Left) / static_cast<float>(m_PixelsX);
		float pixelHeight = (m_Top + m_Bottom) / static_cast<float>(m_PixelsY);

		float horizontalOffset = static_cast<float>(pixelX) * pixelWidth;
		float verticalOffset = static_cast<float>(pixelY) * pixelHeight;

		// Use offset in camera coordinates to find how much in each direction to move
		auto pixelCoordinates = topLeftCorner + 
								m_Camera.GetU() * horizontalOffset - 
								m_Camera.GetW() * verticalOffset;

		// Now depending on the ray type, we find a different position inside the pixel
		glm::vec3 offsetInsidePixel;

		switch (type)
		{
		case RayType::TOP_LEFT: offsetInsidePixel = glm::vec3(0); break;
		case RayType::MID: offsetInsidePixel = m_Camera.GetU() * pixelWidth / 2.f - m_Camera.GetW() * pixelHeight / 2.f; break;
		default: assert(false && "Ray type not yet implemented");
		}

		// Get coordinates of output point
		pixelCoordinates += offsetInsidePixel;

		// Generate ray 
		return Ray{ m_Camera.GetPosition(), glm::normalize(pixelCoordinates - m_Camera.GetPosition()) };
	}

	// -- < Recursive ray tracer > ----------------------------------------------
	RecursiveRayTracer::RecursiveRayTracer(const SceneDescription& description)
	{
		// Set up scene description 
		m_SceneDescription = description;

		// Set up ray generator
		auto const cam = Camera(description.camera.up, description.camera.position, description.camera.lookAt);
		auto const height = HeightFromAspectRatio(description.imgWidth / description.imgHeight, description.imgWidth);
		m_RayGenerator = RayGenerator{
			cam,
			description.imgWidth,
			height, // ??
			description.imgResX,
			description.imgResY,
			FocalLength(description.camera.fovy,height)
		};

	}

	int RecursiveRayTracer::Draw(FIBITMAP*& outImage)
	{
		// Allocate space for this image
		auto Image = FreeImage_Allocate(
			m_SceneDescription.imgResX, 
			m_SceneDescription.imgResY,
			static_cast<size_t>(8 * 3)); // 8 bit colors, 3 color components

		if (!Image)
			return FAIL;

		// Set up geometry for objects. I think this is unnecessary bc the ray casting should be
		// enough to simulate camera positioning
		SetUpGeometry();

		// Set up SDL window
		SDL_Renderer* renderer = nullptr;
		SDL_Window* window = nullptr;
		SDL_Event event;
		SDL_CreateWindowAndRenderer(m_SceneDescription.imgResX, m_SceneDescription.imgResY, 0, &window, &renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetWindowTitle(window, "Ray Tracer preview");

		// Now we can intersect things with rays
		RGBQUAD color;
		ProgressBar bar(m_SceneDescription.imgResX * m_SceneDescription.imgResY);
		// Use this variables to print a progress bar
		
		for (size_t i = 0; i < m_SceneDescription.imgResY; i++)
		{
			for (size_t j = 0; j < m_SceneDescription.imgResX; j++)
			{
				// Set next pixel to render as white
				SDL_SetRenderDrawColor(renderer, 1, 1, 1, 1);
				SDL_RenderDrawPoint(renderer, i, j);
				SDL_RenderPresent(renderer);

				auto const ray = m_RayGenerator.GetRayThroughPixel(i, j);
				auto result = IntersectRay(ray);

				auto const shadeColor = 255.0f * Shade(result);

				// -- DEBUG ONLY ------------------------
				//glm::vec3 const shadeColor(255.0f * glm::dot(ray.direction, m_RayGenerator.GetCamera().GetV()));
			
				// --------------------------------------
				color.rgbRed = shadeColor.r;
				color.rgbGreen = shadeColor.g;
				color.rgbBlue = shadeColor.b;
				// Write to image
				FreeImage_SetPixelColor(Image, i, j, &color);

				// Update renderer & window
				SDL_SetRenderDrawColor(
					renderer, 
					static_cast<Uint8>(shadeColor.r), 
					static_cast<Uint8>(shadeColor.g), 
					static_cast<Uint8>(shadeColor.b), 
					static_cast<Uint8>(shadeColor.a)
				);

				SDL_RenderDrawPoint(renderer, i, j);

				if (SDL_PollEvent(&event));

				SDL_RenderPresent(renderer);

				// Update bar
				bar.Step();
				bar.Draw();
			}
		}

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		outImage = Image;
		return SUCCESS;
	}

	float RecursiveRayTracer::FocalLength(float fovy, float height)
	{
		float const cos = glm::pow(glm::cos(fovy / 2.f), 2.f);
		float result = cos * height * height;
		result = result / (1 - cos);
		result = glm::sqrt(result);
		
		return result;
	}

	float RecursiveRayTracer::HeightFromAspectRatio(float aspectRatio, float width)
	{
		return width / aspectRatio;
	}

	void RecursiveRayTracer::SetUpGeometry()
	{
		for (auto& obj : m_SceneDescription.GetObjects())
		{
			obj.SetGeometry();
		}
	}

	RayIntersectionResult RecursiveRayTracer::IntersectRay(const Ray& ray)
	{
		// Find nearest object intersecting this ray
		RayIntersectionResult finalResult;
		float nearestT = INFINITY;
		for (auto const &obj : m_SceneDescription.GetObjects())
		{
			auto const result = IntersectRayToObject(ray, obj, 0, nearestT);
			if (result.WasIntersection() && result.t < nearestT && result.t > 0)
			{
				finalResult = result;
				nearestT = result.t;
			}
			
		}

		return finalResult;
	}

	RayIntersectionResult RecursiveRayTracer::IntersectRayToObject(const Ray& ray, const Object& object, float minT, float maxT) const
	{
		switch (object.shape)
		{
		case Shape::Sphere:
			return IntersectRayToSphere(ray, object);
		default:
			return IntersectRayToTesselatedObject(ray, object, minT, maxT);
		}	
	}

	RayIntersectionResult RecursiveRayTracer::IntersectRayToSphere(const Ray& ray, const Object& sphere, float minT, float maxT) const
	{
		// Sanity check 
		assert(sphere.shape == Shape::Sphere);

		// We just have to compue intersection point solving for t in the
		// ecuation of a sphere substituting by a point in the ray

		// Compute discriminant to check what kind of intersection we have here
		auto const d = ray.direction;
		auto const e = ray.position;
		auto const c = glm::vec3 (sphere.transform * glm::vec4(0,0,0,1)); // Extract sphere position from transform
		auto const r = sphere.size; // Radius

		auto discriminant = glm::dot(d, e - c);
		discriminant = discriminant * discriminant;
		discriminant -= (dot(d, d) * dot(e - c, e - c) - r * r);

		float t = -dot(d, e - c);
		std::vector<float> results;
		auto emptyResult = RayIntersectionResult{ nullptr, glm::vec3(0), glm::vec3(0), 0 };
		if (discriminant < -0.0001)
		{
			// no intersection at all
			return emptyResult;
		}

		if (abs(discriminant) > 0.0001) // near 0
		{
			results.emplace_back(t + glm::sqrt(discriminant));
			results.emplace_back(t - glm::sqrt(discriminant));
		}
		else
		{
			results.emplace_back(t);
		}

		float desiredResult = INFINITY;
		for (auto const result : results)
		{
			if (result > 0 && result < desiredResult)
			{
				desiredResult = result;
			}
		}

		// Check if t out of ray range
		if (desiredResult < minT && desiredResult > maxT)
			return emptyResult;

		// Compute intersection point and normal
		glm::vec3 const intersectionPos = ray.position + desiredResult * ray.direction;
		glm::vec3 const intersectionNormal = glm::vec3(intersectionPos - c);

		return RayIntersectionResult{
				std::make_shared<const Object*>(&sphere),
				intersectionNormal,
				intersectionPos, desiredResult };
	}

	bool  RecursiveRayTracer::IntersectRayToTriangle(const Ray& ray, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& n1, const glm::vec3& n2, const glm::vec3& n3, glm::vec3& outIntersection, glm::vec3& outNormal, float& outT, float minT, float maxT)
	{
		// Before everything, perform backface culling. If direction and triangle normal
		// have an angle > 90, then it can't intersect by any mean. Winding order is clock wise
		auto const triangleNormal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
		if (glm::dot(ray.direction, triangleNormal) >= 0)
			return false;

		// In this function we use the final result of solving the ec. system
		// of intersecting the ray with the baricentric coordinates of the triangle
		auto const detA = glm::determinant(glm::mat3(v1 - v2, v1 - v3, ray.direction));

		auto const t = glm::determinant(glm::mat3(v1 - v2, v1 - v3, v1 - ray.position)) / detA;

		// Check if intersection point is in valid range
		if (t < minT || t > maxT)
			return false;

		// Check if beta in valid range
		auto const beta = glm::determinant(glm::mat3(v1 - v2, v1 - ray.position, ray.direction)) / detA;
		if (beta < 0 || beta > 1)
			return false;

		// Check if alpha in valid range
		auto const alpha = glm::determinant(glm::mat3(v1 - ray.position, v1 - v3, ray.direction)) / detA;
		if (alpha < 0 || alpha + beta > 1)
			return false;

		// Now that we actually hit the triangle, we need to compute the normals.
		// We do so by interpolating the normals of the other 3 vertices.
		// We have to find a,b,c such that
		// intersectionPoint = a * v1 + b * v2 + c * v3
		// and then we will use these numbers such that
		// newNormal = a * n1 + b * n2 + c * n3.
		// And therefore we have to solve for
		// [v1 v2 v3] * [a b c]' = intersectionPoint

		auto const intersectionPoint = ray.position + t * ray.direction;
		auto vecMatrix = glm::mat3(v1, v2, v3);
		auto const vecMatrixDet = glm::determinant(vecMatrix);

		// Compute a
		vecMatrix[0] = intersectionPoint;
		auto const a = glm::determinant(vecMatrix) / vecMatrixDet;

		// Compute b
		vecMatrix[0] = v1;
		vecMatrix[1] = intersectionPoint;
		auto const b = glm::determinant(vecMatrix) / vecMatrixDet;

		// Compute c
		vecMatrix[1] = v2;
		vecMatrix[2] = intersectionPoint;
		auto const c = glm::determinant(vecMatrix) / vecMatrixDet;

		// compute normal
		auto const newNormal = a * n1 + b * n2 + c * n3;

		// Return results
		outNormal = newNormal;
		outIntersection = intersectionPoint;
		outT = t;

		return true;
	}

	RayIntersectionResult RecursiveRayTracer::IntersectRayToTesselatedObject(const Ray& ray, const Object& object, float minT, float maxT) const
	{
		// sanity check
		assert(object.shape != Shape::Sphere && "Sphere is parametric object");

		glm::vec3 intersection, normal;
		float t = maxT;
		bool hitSome = false;
		for (auto const& triIndices : object.geometry.indices)
		{
			glm::vec3 v1, v2, v3;
			v1 = object.geometry.vertices[triIndices.x];
			v2 = object.geometry.vertices[triIndices.y];
			v3 = object.geometry.vertices[triIndices.z];

			glm::vec3 n1, n2, n3;
			n1 = object.geometry.normals[triIndices.x];
			n2 = object.geometry.normals[triIndices.y];
			n3 = object.geometry.normals[triIndices.z];

			glm::vec3 nextIntersect, nextNormal;
			float nextT;
			bool wasIntersection = IntersectRayToTriangle(ray, v1, v2, v3, n1, n2, n3, nextIntersect, nextNormal, nextT, minT, t);

			// Continue if no intersection
			if (!wasIntersection)
				continue;

			if (nextT > minT && nextT < t)
			{
				hitSome = hitSome || wasIntersection;
				t = nextT;
				intersection = nextIntersect;
				normal = nextNormal;
			}
		}

		// Check if there was a hit
		if (hitSome)
		{
			return RayIntersectionResult{
			std::make_shared<const Object*>(&object),
				normal,
				intersection,
				t };
		}
	
		return RayIntersectionResult{nullptr, glm::vec3(0), glm::vec3(0), 0 };
	}

	glm::vec4 RecursiveRayTracer::Shade(const RayIntersectionResult& rayIntersection)
	{

		if (rayIntersection.WasIntersection())
			//return (*rayIntersection.object)->diffuse;
			return glm::vec4(1);
		else
			return glm::vec4(0);
	}

	// -- <Progres bar> ------------------------------------------------------------------------------------------
	void ProgressBar::Step()
	{
		m_CurrentSteps = std::min(m_CurrentSteps + 1, m_NSteps);
	}

	void ProgressBar::Draw() const
	{
		float const progress = static_cast<float>(m_CurrentSteps) / static_cast<float>(m_NSteps);
		std::cout << "[";
		auto const pos = static_cast<size_t>((static_cast<float>(m_BarSize) * progress));
		for (size_t i = 0; i < m_BarSize; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << static_cast<size_t>(progress * 100.0) <<  "(" << m_CurrentSteps << " / " << m_NSteps << ")" " %\r";
		std::cout.flush();
	}
}