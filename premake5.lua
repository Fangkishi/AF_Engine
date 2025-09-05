include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "AFEngine"
	architecture "x86_64"
	startproject "AF-Editor"

	filter "system:windows"
		buildoptions { "/utf-8" }

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	include "AFEngine/vendor/Box2D"
	include "AFEngine/vendor/GLFW"
	include "AFEngine/vendor/Glad"

	include "AFEngine/vendor/imgui"
	include "AFEngine/vendor/yaml-cpp"
group ""

group "Core"
	include "AFEngine"
group ""

group "Tools"
	include "AF-Editor"
group ""

group "Misc"
	include "Sandbox"
group ""
