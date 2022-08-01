// Local includes
#include "RecRays.h"
#include "SceneParser.h"

// C++ includes
#include <sstream>
#include <fstream>
#include <stack>
#include <assert.h>

// External includes
#include <glm/glm.hpp>

namespace RecRays
{
	int SceneParser::Parse(const std::string& filepath, SceneDescription& outDescription)
	{
		// Stack of transform to properly set up objects
		std::stack<glm::mat4> transformStack;
		transformStack.push(glm::mat4(1.0));

		// New scene where data will be stored
		SceneDescription description;

		// Open and read file line by line
		std::ifstream infile(filepath);
		std::string line;
		while (std::getline(infile, line))
		{
			// find first meaningful char
			auto first_pos = line.find_first_not_of(" \t\r\n");

			// If end of line or comment, skip
			if (first_pos == std::string::npos || line[first_pos] == '#')
				continue;

			// Otherwise, read command (first word of string)
			std::stringstream ss(line);
			std::string command;
			ss >> command;

			// Parse according to the command
			if (command == "light")
			{
				float lightPosX, lightPosY, lightPosZ, lightPosW;
				float lightColorX, lightColorY, lightColorZ, lightColorW;

				ss	>> lightPosX >> lightPosY >> lightPosZ >> lightPosW
					>> lightColorX >> lightColorY >> lightColorZ >> lightColorW;

				if (ss.fail())
				{
					assert(false && "Could not parse light command");
				}
			}
			else
			{
				assert(false && "Unrecognized command");
			}

		}


		outDescription = description;
		return SUCCESS;
	}

}
