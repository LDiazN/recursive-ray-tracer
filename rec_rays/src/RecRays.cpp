#include <RecRays.h>
#include <FreeImage.h>

namespace RecRays
{
	int Client::ParseArgs(int argc, char** argv, std::string& outParsedFilepath)
	{
		if (argc < 2)
		{
			std::cerr << "Missing argument: file path to scene description" << std::endl;
			return FAIL;
		}

		// Create filepath
		std::string filepath(argv[1]);

		// Check filepath existence
		std::filesystem::path path(filepath);

		if (!exists(path))
		{
			std::cerr << "Error: file '" << path << "' does not exists." << std::endl;
			return FAIL;
		}

		outParsedFilepath = filepath;
		return SUCCESS;
	}

	int Client::Run()
	{
		std::cout << "Starting RecRays..." << std::endl;
		Init();



		
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
