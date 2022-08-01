#pragma once
#include "RecursiveRayTracer.h"
#include <string>


namespace RecRays
{
	class SceneParser
	{
		SceneParser();

		/**
		 * \brief Try to parse a scene from file
		 * \param filepath Filepath to scene file to read from
		 * \param outDescription generated description if everything went ok
		 * \return Success status, 0 for success, 1 for failure
		 */
		static int Parse(const std::string& filepath, SceneDescription& outDescription);

	private:
		std::vector<float> ParseNNumbers(std::stringstream& ss, int numArgs)
		{
			std::vector<float> result;

			for (int i = 0; i < numArgs; i++)
			{
				float value;
				ss >> value;

				result.push_back(value);
			}
		}
	};
}
