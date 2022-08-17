// stl includes
#include <iostream>

// Local includes
#include <RecRays.h>
#include <FreeImage.h>
#include "SceneParser.h"
#include "RecursiveRayTracer.h"

namespace RecRays
{
	int Client::ParseArgs(int argc, char** argv, std::string& outParsedFilepath)
	{
		if (argc < 2)
		{
			std::cerr << "Missing argument: file path to scene description" << std::endl;
			return FAIL;
		}

		// parse filepath from arguments
		std::string filepath(argv[1]);

		// Check filepath existence
		std::filesystem::path path(filepath);

		if (!exists(path))
		{
			std::cerr << "Error: file '" << path << "' does not exists." << std::endl;
			return FAIL;
		}

		outParsedFilepath = absolute(path).string();
		return SUCCESS;
	}

	int Client::Run()
	{
		std::cout << "Starting RecRays..." << std::endl;
		Init();

		// Try to parse scene
		std::cout << "Parsing scene from " << m_SceneFile << "..." << std::endl;

		SceneDescription scene;
		auto status = SceneParser::Parse(m_SceneFile, scene);
		if (status != SUCCESS)
		{
			std::cerr << "[ERROR] Could not parse scene" << std::endl;
			return FAIL;
		}

		// With a parsed scene, define the recursive ray tracer and generate image
		RecursiveRayTracer rayTracer(scene);

		FIBITMAP* image;
		status = rayTracer.Draw(image);

		if (status != SUCCESS)
		{
			std::cerr << "[ERROR] Could not draw image" << std::endl;
			return FAIL;
		}

		// Save image to file
		std::filesystem::path outputPath("output.png");
		std::cout << "Saving image to " << absolute(outputPath) << "..." << std::endl;
		if (FreeImage_Save(FIF_PNG, image, outputPath.string().c_str()))
			std::cout << "Image successfully saved!" << std::endl;
		else
			std::cerr << "ERROR: Could not save image :(" << std::endl;
		
		
		std::cout << "Shutting Down RecRays..." << std::endl;
		Shutdown();
		return SUCCESS;
	}

	void Client::Init()
	{
		std::cout << "Starting FreeImage..." << std::endl;
		FreeImage_Initialise();
	}

	void Client::Shutdown()
	{
		std::cout << "Shutting down FreeImage..." << std::endl;
		FreeImage_DeInitialise();
	}



}
