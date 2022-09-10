workspace "RecRays"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder 
IncludeDir = {}
IncludeDir["glm"]   = "rec_rays/vendor/glm"
IncludeDir["freeimage"] = "rec_rays/vendor/freeimage/Source"
IncludeDir["sdl"] = "rec_rays/vendor/sdl/include"
IncludeDir["threadpool"] = "rec_rays/vendor/threadpool"

include "rec_rays/vendor/freeimage"
include "rec_rays/vendor/sdl"

project "rec_rays"
	location "rec_rays"
	kind "ConsoleApp"
	language "C++"
	staticruntime "on"
	cppdialect "C++17"


	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files	
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.threadpool}/**.cpp" -- Threadpool dependency
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"rec_rays/src", --should not work
		"%{IncludeDir.glm}",
		"%{IncludeDir.freeimage}",
		"%{IncludeDir.sdl}",
		"%{IncludeDir.threadpool}"
	}

	links 
	{
		"SDL",
		"SDLmain",

		"FreeImage",
		"FreeImaged.dll",
		"FreeImageLib",
		"FreeImagePlus",
		"LibJPEG",
		"LibPNG",
		"ZLib",
		"OpenEXR",
		"LibOpenJPEG",
		"LibRawLite",
		"LibTIFF4",
		"LibWebP",
		"LibJXR"
	}

	filter	"system:windows" -- ran only when compiling on windows
		systemversion "latest"

		defines {			-- define this symbols when building
			"RRAYS_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines {"RRAYS_DEBUG", "RRAYS_ENABLE_ASSERTS"}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RRAYS_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "RRAYS_DIST"
		runtime "Release"
		optimize "on"

	filter "system:windows"
		postbuildcommands {
			"{COPYFILE} vendor/freeimage/Dist/x64/FreeImaged.dll ../bin/Debug-windows-x86_64/rec_rays" ,
			"{COPYFILE} ../x64/Debug/SDL2.dll ../bin/Debug-windows-x86_64/rec_rays"
		}