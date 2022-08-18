#include "Geometry.h"
#include <iostream>

namespace RecRays
{
	void GeometryLoader::Init()
	{
		LoadCubeGeometry();
		LoadTeapotGeometry();
		s_Initialized = true;
	}

	void GeometryLoader::Shutdown()
	{
		s_CubeGeometry = Geometry();
		s_TeapotGeometry = Geometry();
		s_Initialized = false;
	}

	Geometry GeometryLoader::GetCubeGeometry()
	{
		return s_CubeGeometry;
	}

	Geometry GeometryLoader::GetTeapotGeometry()
	{
		return s_TeapotGeometry;
	}

	void GeometryLoader::LoadTeapotGeometry()
	{
		std::vector<glm::vec3> teapotVertices;
		std::vector<glm::vec3> teapotNormals;
		std::vector<glm::uvec3> teapotIndices;

		FILE* fp;
		float x, y, z;
		int fx, fy, fz, ignore;
		int c1, c2;
		float minY = INFINITY, minZ = INFINITY;
		float maxY = -INFINITY, maxZ = -INFINITY;

		fp = fopen(s_PathToTeapotObj, "rb");

		if (fp == nullptr) {
			std::cerr << "Error loading file: " << s_PathToTeapotObj<< std::endl;
			exit(-1);
		}

		while (!feof(fp)) {
			c1 = fgetc(fp);
			while (!(c1 == 'v' || c1 == 'f')) {
				c1 = fgetc(fp);
				if (feof(fp))
					break;
			}
			c2 = fgetc(fp);

			if ((c1 == 'v') && (c2 == ' ')) {
				fscanf(fp, "%f %f %f", &x, &y, &z);
				teapotVertices.push_back({ x, y, z });
				if (y < minY) minY = y;
				if (z < minZ) minZ = z;
				if (y > maxY) maxY = y;
				if (z > maxZ) maxZ = z;
			}
			else if ((c1 == 'v') && (c2 == 'n')) {
				fscanf(fp, "%f %f %f", &x, &y, &z);
				// Ignore the normals in mytest2, as we use a solid color for the teapot.
				teapotNormals.push_back(glm::normalize(glm::vec3{ x, y, z }));
			}
			else if ((c1 == 'f'))
			{
				fscanf(fp, "%d//%d %d//%d %d//%d", &fx, &ignore, &fy, &ignore, &fz, &ignore);
				teapotIndices.emplace_back(glm::uvec3{fx - 1, fy - 1 , fz - 1 });
			}
		}

		fclose(fp); // Finished parsing

		// Recenter the teapot
		float avgY = (minY + maxY) / 2.0f - 0.02f;
		float avgZ = (minZ + maxZ) / 2.0f;
		for (unsigned int i = 0; i < teapotVertices.size(); ++i) {
			glm::vec3 shiftedVertex = (teapotVertices[i] - glm::vec3(0.0f, avgY, avgZ)) * glm::vec3(1.58f, 1.58f, 1.58f);
			teapotVertices[i] = shiftedVertex;
		}

		s_TeapotGeometry = Geometry{ teapotVertices, teapotNormals, teapotIndices };
	}

	void GeometryLoader::LoadCubeGeometry()
	{
		std::vector<glm::vec3> cubePoints = {
			// Front face
			{ -0.5f, -0.5f, 0.5f },{ -0.5f, 0.5f, 0.5f },{ 0.5f, 0.5f, 0.5f },{ 0.5f, -0.5f, 0.5f },
			// Back face
			{ -0.5f, -0.5f, -0.5f },{ -0.5f, 0.5f, -0.5f },{ 0.5f, 0.5f, -0.5f },{ 0.5f, -0.5f, -0.5f },
			// Left face
			{ -0.5f, -0.5f, 0.5f },{ -0.5f, 0.5f, 0.5f },{ -0.5f, 0.5f, -0.5f },{ -0.5f, -0.5f, -0.5f },
			// Right face
			{ 0.5f, -0.5f, 0.5f },{ 0.5f, 0.5f, 0.5f },{ 0.5f, 0.5f, -0.5f },{ 0.5f, -0.5f, -0.5f },
			// Top face
			{ 0.5f, 0.5f, 0.5f },{ -0.5f, 0.5f, 0.5f },{ -0.5f, 0.5f, -0.5f },{ 0.5f, 0.5f, -0.5f },
			// Bottom face
			{ 0.5f, -0.5f, 0.5f },{ -0.5f, -0.5f, 0.5f },{ -0.5f, -0.5f, -0.5f },{ 0.5f, -0.5f, -0.5f }
		};

		std::vector<glm::vec3> cubeNormals = {
			// Front face
			{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },
			// Back face
			{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, -1.0f },
			// Left face
			{ -1.0f, 0.0f, 0.0f },{ -1.0f, 0.0f, 0.0f },{ -1.0f, 0.0f, 0.0f },{ -1.0f, 0.0f, 0.0f },
			// Right face
			{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },
			// Top face
			{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
			// Bottom face
			{ 0.0f, -1.0f, 0.0f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, -1.0f, 0.0f }
		};

		std::vector<glm::uvec3> cubeIndices = {
			{0, 1, 2}, {0, 2, 3}, // Front face
			{4, 5, 6}, {4, 6, 7}, // Back face
			{8, 9, 10}, {8, 10, 11}, // Left face
			{12, 13, 14}, {12, 14, 15}, // Right face
			{16, 17, 18}, {16, 18, 19}, // Top face
			{20, 21, 22}, {20, 22, 23} // Bottom face
		};

		s_CubeGeometry = Geometry{ cubePoints, cubeNormals, cubeIndices };
	}
}
