#pragma once
#include <string>
#include <iostream>
#include <filesystem>

#define SUCCESS 0
#define FAIL 1

namespace RecRays
{
	/**
	 * \brief Main interface for this application, use this class to run workflow
	 */
	class Client
	{
	public:
		/**
		 * \brief Create a new client object to run workflow
		 * \param filepath Name of file to parse to generate scene
		 */
		Client(const std::string& filepath, size_t width = 512, size_t height = 512)
			: m_SceneFile(filepath)
			, m_Width(width)
			, m_Height(height)
		{}

		/**
		 * \brief Parse arguments from command line arguments
		 * \param argc How many arguments, provided from main function 
		 * \param argv Actual arguments, provided from main function
		 * \param outParsedFilepath Parsed filepath from arguments
		 * \return Success status: 0 for success, 1 for failure
		 */
		static int ParseArgs(int argc, char** argv, std::string& outParsedFilepath);

		/**
		 * \brief Run application: Parse scene file and perform ray tracing algorithm
		 */
		int Run();

	private:
		/**
		 * \brief Init subsystems, like freeimage
		 */
		void Init();

		/**
		 * \brief Shutdown subsystems, 
		 */
		void Shutdown();

	private:
		/**
		 * \brief File where the scene will be parsed from
		 */
		std::string m_SceneFile;

		// Dimensions of image to render
		size_t m_Width, m_Height;

	};
}
