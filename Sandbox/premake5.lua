project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/AFEngine/vendor/spdlog/include",
		"%{wks.location}/AFEngine/src",
		"%{wks.location}/AFEngine/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"AFEngine"
	}

	filter "system:windows"
		systemversion "latest"

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
