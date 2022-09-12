// Local includes
#include "RecursiveRayTracer.h"
#include "RecRays.h"

// STL includes
#include <assert.h>

// Vendor includes
#include <glm/gtc/matrix_transform.hpp>
#include <SDL.h>
#include <threadpool.h>


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
		for (auto& normal : geometry.normals)
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
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			-(m_Position.x - m_PosToLookAt.x), -(m_Position.y - m_PosToLookAt.y), -(m_Position.z - m_PosToLookAt.z), 1.f,
		};

		return axisTransform * inverseTranslation;
	}


	// -- < Ray Generation > ----------------------------------------------

	Ray RayGenerator::GetRayThroughPixel(size_t pixelX, size_t pixelY, RayType type) const
	{
		// Find the top left corner to get position of next pixel from this
		const glm::vec3 topLeftCorner = -m_Left * m_Camera.GetU() + m_Top * m_Camera.GetW() + m_DistanceToViewPlane * m_Camera.GetV();

		// Use pixel width and height to find how much to offset for each step
		const float pixelWidth = (m_Right + m_Left) / static_cast<float>(m_PixelsX);
		const float pixelHeight = (m_Top + m_Bottom) / static_cast<float>(m_PixelsY);

		const float horizontalOffset = static_cast<float>(pixelX) * pixelWidth;
		const float verticalOffset = static_cast<float>(pixelY) * pixelHeight;

		// Use offset in camera coordinates to find how much in each direction to move
		auto pixelCoordinates = topLeftCorner +
			m_Camera.GetU() * horizontalOffset -
			m_Camera.GetW() * verticalOffset;

		// Now depending on the ray type, we find a different position inside the pixel
		glm::vec3 offsetInsidePixel(0);

		float randomX;
		float randomY;

		switch (type)
		{
		case RayType::TOP_LEFT: break;
		case RayType::MID: offsetInsidePixel = m_Camera.GetU() * pixelWidth / 2.f - m_Camera.GetW() * pixelHeight / 2.f; break;
		case RayType::RANDOMIZED:
			// Add random offset inside pixel
			randomX = pixelWidth * static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randomY = pixelHeight * static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			offsetInsidePixel = m_Camera.GetU() * randomX + m_Camera.GetV() * randomY;
			break; 
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

		// Set up random seed to be used in ray generation
		srand(static_cast<unsigned>(time(nullptr)));

	}

	int RecursiveRayTracer::Draw(FIBITMAP*& outImage, size_t nThreads = 12)
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

		// Concurrency stuff: Render disjoint segments of the screen in multiple threads
		thread_pool threads(nThreads);
		std::vector<std::future<void>> futures;

		// Where the colors are actually drawn
		TwoDimensionVector<glm::vec4> colorBuffer(m_SceneDescription.imgResX, m_SceneDescription.imgResY);

		// Start parallel shading: Schedule regions for each thread
		uint32_t const yIntervalSize = m_SceneDescription.imgResY / nThreads;
		for (uint32_t i = 0; i < nThreads; i++)
		{
			futures.push_back(threads.execute(&RecursiveRayTracer::DrawThread, this, std::ref(colorBuffer), 0, m_SceneDescription.imgResX, i * yIntervalSize, (i + 1) * yIntervalSize));
		}


		// Now we can intersect things with rays
		RGBQUAD color;
		// Use this variables to print a progress bar

		// Draw in SDL while futures are not yet done 
		// Set up SDL window
		SDL_Renderer* renderer = nullptr;
		SDL_Window* window = nullptr;
		SDL_Event event;
		SDL_CreateWindowAndRenderer(m_SceneDescription.imgResX, m_SceneDescription.imgResY, 0, &window, &renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetWindowTitle(window, "Ray Tracer preview");

		bool ready = false;

		while (!ready)
		{
			for (size_t i = 0; i < m_SceneDescription.imgResY; i++)
			{
				for (size_t j = 0; j < m_SceneDescription.imgResX; j++)
				{
					glm::vec4 shadeColor = 255.0f * colorBuffer.Get(i, j);
					shadeColor.r = glm::clamp(shadeColor.r, 0.f, 255.f);
					shadeColor.g = glm::clamp(shadeColor.g, 0.f, 255.f);
					shadeColor.b = glm::clamp(shadeColor.b, 0.f, 255.f);

					// Update renderer & window
					SDL_SetRenderDrawColor(
						renderer,
						static_cast<Uint8>(shadeColor.r),
						static_cast<Uint8>(shadeColor.g),
						static_cast<Uint8>(shadeColor.b),
						255
					);

					SDL_RenderDrawPoint(renderer, i, j);

					// Update bar
				}
			}

			if (SDL_PollEvent(&event)); // do nothing with event

			// Display image
			SDL_RenderPresent(renderer);

			// Update ready status
			bool allEnded = true;
			for (auto const& future : futures)
			{
				if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				{
					allEnded = false;
					break;
				}
			}
			ready = allEnded;
		}

		// Draw final FreeImage output image
		for (size_t i = 0; i < m_SceneDescription.imgResY; i++)
		{
			for (size_t j = 0; j < m_SceneDescription.imgResX; j++)
			{
				glm::vec4 shadeColor = 255.0f * colorBuffer.Get(i, j);

				shadeColor.r = glm::clamp(shadeColor.r, 0.f, 255.f);
				shadeColor.g = glm::clamp(shadeColor.g, 0.f, 255.f);
				shadeColor.b = glm::clamp(shadeColor.b, 0.f, 255.f);

				color.rgbRed = shadeColor.r;
				color.rgbGreen = shadeColor.g;
				color.rgbBlue = shadeColor.b;

				FreeImage_SetPixelColor(Image, i, j, &color);
			}
		}

		for (auto& future : futures)
			future.get(); // end all threads

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		outImage = Image;
		return SUCCESS;
	}

	void RecursiveRayTracer::DrawThread(TwoDimensionVector<glm::vec4>& outBuffer, size_t startI, size_t endI, size_t startJ, size_t endJ)
	{
		for (size_t i = startI; i < endI; i++)
		{
			for (size_t j = startJ; j < endJ; j++)
			{

				auto const ray = m_RayGenerator.GetRayThroughPixel(i, j);
				auto const result = IntersectRay(ray);
				auto shadeColor = Shade(result);

				outBuffer.Set(i, j, shadeColor);
			}
		}
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

	RayIntersectionResult RecursiveRayTracer::IntersectRay(const Ray& ray, float minT, float maxT)
	{
		// Find nearest object intersecting this ray
		RayIntersectionResult finalResult;
		float nearestT = maxT;
		for (auto const& obj : m_SceneDescription.GetObjects())
		{
			auto const result = IntersectRayToObject(ray, obj, minT, nearestT);
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

		// We just have to compute intersection point solving for t in the
		// equation of a sphere substituting by a point in the ray

		// Compute discriminant to check what kind of intersection we have here
		auto const d = ray.direction;
		auto const e = ray.position;
		auto const c = glm::vec3(sphere.transform * glm::vec4(0, 0, 0, 1)); // Extract sphere position from transform
		auto const r = sphere.size; // Radius

		auto discriminant = glm::dot(d, e - c);
		discriminant = discriminant * discriminant;
		discriminant -= (dot(d, d) * dot(e - c, e - c) - r * r);

		float t = -dot(d, e - c);
		std::vector<float> results;
		auto emptyResult = RayIntersectionResult{ nullptr, glm::vec3(0), glm::vec3(0), 0 , ray };
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
		glm::vec3 const intersectionNormal = glm::normalize(glm::vec3(intersectionPos - c));

		return RayIntersectionResult{
				std::make_shared<const Object*>(&sphere),
				intersectionNormal,
				intersectionPos, desiredResult, ray };
	}

	bool RecursiveRayTracer::PointInsideTriangle(const glm::vec3& point, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
	{
		auto const edge1 = v2 - v1;
		auto const edge2 = v3 - v2;
		auto const edge3 = v1 - v3;

		auto const v12P = point - v1;
		auto const v22P = point - v2;
		auto const v32P = point - v3;

		auto const normal = glm::cross(v2 - v1, v3 - v1);

		return	glm::dot(normal, glm::cross(edge1, v12P)) > 0 &&
				glm::dot(normal, glm::cross(edge2, v22P)) > 0 &&
				glm::dot(normal, glm::cross(edge3, v32P)) > 0
		;
	}

	bool  RecursiveRayTracer::IntersectRayToTriangle(const Ray& ray, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& n1, const glm::vec3& n2, const glm::vec3& n3, glm::vec3& outIntersection, glm::vec3& outNormal, float& outT, float minT, float maxT)
	{
		// Before everything, perform backface culling. If direction and triangle normal
		// have an angle > 90, then it can't intersect by any mean. Winding order is clock wise
		auto const triangleNormal = glm::cross(v2 - v1, v3 - v1);
		if (glm::dot(ray.direction, triangleNormal) >= 0)
			return false;

		constexpr float EPSILON = 0.0000001f;

		auto const edge1 = v2 - v1;
		auto const edge2 = v3 - v1;
		auto const h = glm::cross(ray.direction, edge2);
		auto const k = glm::dot(edge1, h);

		// No intersection, ray is parallel to triangle
		if (k > -EPSILON && k < EPSILON)
			return false;

		auto const f = 1.f / k;
		auto const s = ray.position - v1;
		auto const u = f * glm::dot(s, h);
		if (u < 0.0f || u > 1.0)
			return false;

		auto const q = glm::cross(s, edge1);
		auto const v = f * glm::dot(ray.direction, q);
		if (v < 0.0 || u + v > 1.0)
			return false;

		auto const t = f * glm::dot(edge2, q);
		if (t <= EPSILON)
			return false; // Line intersection, but not ray

		auto const intersectionPoint = ray.position + ray.direction * t;

		// Compute normal based on intersection position
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

			if (nextT >= minT && nextT < t)
			{
				hitSome = true;
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
				t,
				ray
			};
		}
	
		return RayIntersectionResult{nullptr, glm::vec3(0), glm::vec3(0), 0 };
	}

	glm::vec4 RecursiveRayTracer::Shade(const RayIntersectionResult& rayIntersection, uint32_t maxRecursionDepth)
	{
		// If no intersection, do nothing and return black
		if (!rayIntersection.WasIntersection())
			return glm::vec4(0);


		auto normal = glm::normalize(rayIntersection.normal);
		glm::vec4 lightColor(0);
		auto const &object = **rayIntersection.object;
		// Add ambient color
		lightColor += object.ambient;

		// Compute diffuse + specular for each light
		glm::vec4 diffuse(0), specular(0);
		for (auto const & light : m_SceneDescription.GetLights())
		{
			// Compute direction of light. If point light, then use relative position.
			// If directional light, use straight up as direction.
			glm::vec3 lightDirection(0);
			float maxRayToLightLen;
			if (light.position.w == 0.0)
			{
				lightDirection = glm::normalize(light.position);
				maxRayToLightLen = INFINITY;
			}
			else
			{
				lightDirection = glm::vec3(light.position) - rayIntersection.position;
				maxRayToLightLen = glm::length(lightDirection);
				lightDirection = lightDirection / maxRayToLightLen;
			}

			// Check if light can reach this point 
			Ray ray{
				rayIntersection.position + normal* 0.01f, lightDirection
			};

			auto const result = IntersectRay(ray, 0.f, maxRayToLightLen);
			if (result.WasIntersection())
				continue; // Light is occluded, so nothing more to add

			// Compute diffuse 
			diffuse +=
				object.diffuse *
				light.color *
				glm::max(0.f, glm::dot(glm::vec3(normal), lightDirection));

			// Compute specular
			const glm::vec3 halfVec = glm::normalize(
					lightDirection + 
					(
						-rayIntersection.ray.direction
					)
			);

			specular += glm::pow(
						glm::max(
							0.f, 
							glm::dot(glm::vec3(normal), halfVec)
							), 
						object.shininess
						) * 
						light.color * 
						object.specular;

		}

		lightColor += diffuse + specular;
		lightColor.a = 1;

		if (maxRecursionDepth == 0)
			return lightColor;

		// Now compute reflections.
		// Generate reflection vector: r = d - 2(dot(d, normal)) * normal

		auto const& d = rayIntersection.ray.direction;
		const glm::vec3 reflectionDir = d - 2.f * (glm::dot(d, normal)) * normal;
		const Ray reflectionRay{ rayIntersection.position + normal * 0.01f, reflectionDir };
		auto const reflecResult = IntersectRay(reflectionRay);

		// If nothing to reflect, just return your own color
		if (!reflecResult.WasIntersection())
			return lightColor;

		auto const reflecColor = object.mirror * Shade(rayIntersection, maxRecursionDepth - 1);
		lightColor += reflecColor;
		lightColor.a = 1;
		return lightColor;
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
		std::cout << "] " << static_cast<size_t>(progress * 100.0) <<  " %(" << m_CurrentSteps << " / " << m_NSteps << ")" "\r";
		std::cout.flush();
	}
}