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
	void Object::SetGeometry(const glm::mat4& ViewMatrix)
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
		}
		// TODO terminar esta función para que termine de transformar los vertices al formato apropiado
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
		// TODO terminar esta implementación de camara (no esta función) y crear el generador de rayos
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

}