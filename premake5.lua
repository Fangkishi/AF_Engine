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
IncludeDir["glm"] = "AFEngine/vendor/glm"
IncludeDir["stb_image"] = "AFEngine/vendor/stb_image"

include "AFEngine/vendor/GLFW"
include "AFEngine/vendor/GLad"
include "AFEngine/vendor/imgui"

project "AFEngine"
	location "AFEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "afpch.h"
	pchsource "AFEngine/src/afpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
	}

	links
	{
		"GLFW",
		"GLad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"AF_PLATFORM_WINDOWS",
			"AF_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "AF_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "AF_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "AF_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"AFEngine/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLad}",
		"%{IncludeDir.ImGui}",
	}

	links
	{
		"AFEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"AF_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "AF_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "AF_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "AF_DIST"
		runtime "Release"
		optimize "on"