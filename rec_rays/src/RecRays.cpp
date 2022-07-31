#include <RecRays.h>

namespace RecRays
{
	int Client::Run()
	{
		std::cout << "Starting RecRays..." << std::endl;

		return SUCCESS;
	}

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

}
