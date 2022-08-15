#include "RecursiveRayTracer.h"
#include "RecRays.h"

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
		auto v = glm::normalize(m_PosToLookAt - m_Position);
		auto u = glm::normalize(glm::cross(v, m_Up));
		auto w = glm::normalize(glm::cross(u, v));

		m_U = u;
		m_V = v;
		m_W = w;
		// TODO terminar esta implementación de camara (no esta función) y crear el generador de rayos
	}
}