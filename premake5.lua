workspace "AFEngine"
	architecture "x64"
	startproject "Sandbox"

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
IncludeDir["GLad"] = "AFEngine/vendor/GLad/include"
IncludeDir["ImGui"] = "AFEngine/vendor/imgui"

include "AFEngine/vendor/GLFW"
include "AFEngine/vendor/GLad"
include "AFEngine/vendor/imgui"

project "AFEngine"
	location "AFEngine"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

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
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLad}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"GLFW",
		"GLad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "10.0"

		defines
		{
			"AF_PLATFORM_WINDOWS",
			"AF_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}

	filter "configurations:Debug"
		defines "AF_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "AF_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "AF_DIST"
		runtime "Release"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

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
		systemversion "10.0"

		defines
		{
			"AF_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "AF_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "AF_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "AF_DIST"
		runtime "Release"
		optimize "On"