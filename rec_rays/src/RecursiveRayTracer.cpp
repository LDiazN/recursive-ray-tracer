#include "RecursiveRayTracer.h"
#include "RecRays.h"
#include <assert.h>
#include <glm/gtc/matrix_transform.hpp>

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
		return Ray{ m_Camera.GetPosition(), pixelCoordinates - m_Camera.GetPosition() };
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

		// Now we can intersect things with rays
		for (size_t i = 0; i < m_SceneDescription.imgResX; i++)
		{
			for (size_t j = 0; i < m_SceneDescription.imgResY; i++)
			{
				auto const ray = m_RayGenerator.GetRayThroughPixel(i, j);
				// TODO handle ray-object intersection here
			}
		}

		// -- < TODO SAMPLE CODE, DELETE LATER AND REPLACE WITH ACTUAL COLORING > -----
		RGBQUAD color;
		auto WIDTH = m_SceneDescription.imgResX;
		auto HEIGHT = m_SceneDescription.imgResY;

		for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				color.rgbRed = 0;
				color.rgbGreen = (double)i / WIDTH * 255.0;
				color.rgbBlue = (double)j / HEIGHT * 255.0;

				FreeImage_SetPixelColor(Image, i, j, &color);
			}
		}
		// --------------------------------------------------------------------------

		// TODO: Terminar el ray tracer
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
			auto const result = IntersectRayToObject(ray, obj);
			if (result.WasIntersection() && result.t < nearestT && result.t > 0)
			{
				finalResult = result;
				nearestT = result.t;
			}
			
		}

		return finalResult;
	}

	RayIntersectionResult RecursiveRayTracer::IntersectRayToObject(const Ray& ray, const Object& object) const
	{
		switch (object.shape)
		{
		case Shape::Sphere:
			return IntersectRayToSphere(ray, object);
			default:
				assert(false && "Not yet implemented");
		}	
	}

	RayIntersectionResult RecursiveRayTracer::IntersectRayToSphere(const Ray& ray, const Object& sphere) const
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
		if (discriminant < -0.0001)
		{
			// no intersection at all
			return RayIntersectionResult{nullptr, glm::vec3(0), glm::vec3(0), 0};
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

		float minT = INFINITY;
		for (auto const result : results)
		{
			if (result > 0 && result < minT)
			{
				minT = result;
			}
		}

		// Compute intersection point and normal
		glm::vec3 const intersectionPos = ray.position + minT * ray.direction;
		glm::vec3 const intersectionNormal = glm::vec3(intersectionPos - c);



		return RayIntersectionResult{
				std::make_shared<const Object*>(&sphere),
				intersectionNormal,
				intersectionPos, minT };
	}
}