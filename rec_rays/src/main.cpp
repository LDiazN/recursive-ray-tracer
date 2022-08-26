#include <iostream>
#include <FreeImage.h>
#include <glm/glm.hpp>
#include "RecRays.h"
#include <SDL.h>

int main(int argc, char ** argv)
{

    
    // Parse arguments for client object
    std::string sceneFile;
	int status = RecRays::Client::ParseArgs(argc, argv, sceneFile);

    // Finish if could not parse arguments
    if (status == FAIL)
    {
        std::cerr << "Error: Invalid arguments" << std::endl;
        return 1;
    }

    // Create client with valid arguments otherwise
    RecRays::Client client(sceneFile);

    status = client.Run();

    if (status == FAIL)
    {
        std::cerr << "Failed program execution" << std::endl;

        return 1;
    }

    return 0;
}
