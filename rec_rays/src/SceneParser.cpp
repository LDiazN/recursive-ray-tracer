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
#include <glm/gtc/matrix_transform.hpp>

namespace RecRays
{
	int SceneParser::Parse(const std::string& filepath, SceneDescription& outDescription)
	{
		// Stack of transform to properly set up objects
		std::stack<glm::mat4> transformStack;
		transformStack.push(glm::mat4(1.0));

		// New scene where data will be stored
		SceneDescription description;

		// Next object to add
		Object nextObject;

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
				auto const nums = ParseNNumbers(ss, 8);

				Light newLight{
					glm::vec4(nums[0], nums[1], nums[2], nums[3]),
					glm::vec4(nums[4], nums[5], nums[6], nums[7])
				};

				auto couldAdd = description.AddLight(newLight);
				assert(couldAdd == SUCCESS && "Could not add light: too many lights");
			}
			else if (command == "ambient")
			{
				auto const nums = ParseNNumbers(ss, 4);
				nextObject.ambient = glm::vec4(nums[0], nums[1], nums[2], nums[3]);
			}
			else if (command == "diffuse")
			{
				auto const nums = ParseNNumbers(ss, 4);
				nextObject.diffuse = glm::vec4(nums[0], nums[1], nums[2], nums[3]);
			}
			else if (command == "specular")
			{
				auto const nums = ParseNNumbers(ss, 4);
				nextObject.ambient = glm::vec4(nums[0], nums[1], nums[2], nums[3]);
			}
			else if (command == "shininess")
			{
				auto const nums = ParseNNumbers(ss, 1);
				nextObject.shininess = nums[0];
			}
			else if (command == "size")
			{
				auto const nums = ParseNNumbers(ss, 1);
				nextObject.size = nums[0];
			}
			else if (command == "camera")
			{
				auto const nums = ParseNNumbers(ss, 10);
				CameraDescription camera{
					glm::vec3(nums[0], nums[1], nums[2]),
					glm::vec3(nums[3], nums[4], nums[5]),
					glm::vec3(nums[6], nums[7], nums[8]),
					nums[9]
				};
				description.camera = camera;
			}
			else if (command == "sphere" || command == "cube" || command == "teapot")
			{
				// Object pushing: Push current object into scene
				nextObject.transform = transformStack.top();
				auto const nums = ParseNNumbers(ss, 1);

				// Set up object type
				if (command == "sphere")
					nextObject.shape = Shape::Sphere;
				else if (command == "cube")
					nextObject.shape = Shape::Cube;
				else if (command == "teapot")
					nextObject.shape = Shape::Teapot;

				nextObject.size = nums[0];
				description.AddObject(nextObject);
				nextObject = Object();
			}
			else if (command == "translate")
			{
				auto const nums = ParseNNumbers(ss, 3);
				glm::vec3 newTranslate(nums[0], nums[1], nums[2]);

				// Alter top transform
				auto& topTransform = transformStack.top();
				topTransform = glm::translate(topTransform, newTranslate);
			}
			else if (command == "scale")
			{
				auto const nums = ParseNNumbers(ss, 3);
				glm::vec3 newScale(nums[0], nums[1], nums[2]);

				// Alter top transform
				auto& topTransform = transformStack.top();
				topTransform = glm::scale(topTransform, newScale);
			}
			else if (command == "rotate")
			{
				auto const nums = ParseNNumbers(ss, 4);
				glm::vec3 rotationAxis(nums[0], nums[1], nums[2]);
				float rotationDegrees = nums[3];

				auto& topTransform = transformStack.top();
				topTransform = glm::rotate(topTransform, glm::radians(rotationDegrees), rotationAxis);
			}
			else if (command == "pushTransform") // Save current transform by pushing a copy
				transformStack.push(transformStack.top());
			else if (command == "popTransform")
				if (transformStack.size() <= 1)
					std::cerr << "No transform in stack, can't pop transform" << std::endl;
				else
					transformStack.pop();
			else if (command == "image")
			{
				// image width height resX resY
				auto const nums = ParseNNumbers(ss, 5);

				description.imgWidth = nums[0];
				description.imgHeight = nums[1];
				description.imgResX = nums[2];
				description.imgResY = nums[3];
				description.imgDistanceToViewplane = nums[4];
			}
			else
			{
				std::cerr << "Unrecognized command: " << command << std::endl;
				assert(false);
			}
		}

		outDescription = description;
		return SUCCESS;
	}

	std::vector<float> SceneParser::ParseNNumbers(std::stringstream& ss, int numArgs)
	{
		std::vector<float> result(numArgs);

		for (int i = 0; i < numArgs; i++)
		{
			float value;
			assert(ss >> value && "Could no read float value");

			result[i] = value;
		}

		return result;
	}


}
