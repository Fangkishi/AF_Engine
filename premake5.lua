workspace "AFEngine"
	architecture "x64"

	configurations
	{ 
		"Debug", 
		"Release" ,
		"Dist"
	}

	filter "system:windows"
		buildoptions { "/utf-8" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "AFEngine/vendor/GLFW/include"

include "AFEngine/vendor/GLFW"

project "AFEngine"
	location "AFEngine"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "afpch.h"
	pchsource "AFEngine/src/afpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

		defines
		{
			"AF_PLATFORM_WINDOWS",
			"AF_BUILD_DLL",
		}

		postbuildcommands
		{
			"{MKDIR} ../bin/" .. outputdir .. "/Sandbox",
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "AF_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "AF_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "AF_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"AFEngine/vendor/spdlog/include",
		"AFEngine/src"
	}

	links
	{
		"AFEngine"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

		defines
		{
			"AF_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "AF_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "AF_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "AF_DIST"
		optimize "On"