add_rules("mode.debug", "mode.release")

-- Offficial Repos
add_requires("glfw", "glm", "stb") 
add_requires("imgui v1.89-docking", { configs = {glfw_vulkan = true}})

-- Private Repos
add_repositories("myrepo myrepo")
add_requires("loguru", "emilib", "lve")

target("WalnutApp")
    set_kind("binary")
    set_languages("cxx17")
    -- set_pcxxheader("src/pch.h")
    add_files("src/*.cpp|src/pch.cpp")
    add_files("walnut/**.cpp")
    add_defines("WL_PLATFORM_WINDOWS", "WIN32", "IMGUI_VULKAN_DEBUG_REPORT")
    add_includedirs("src", "walnut", path.join(os.getenv("VULKAN_SDK"), "Include"))
    add_packages("glfw", "glm", "stb", "imgui", "loguru", "lve")
    add_linkdirs(path.join(os.getenv("VULKAN_SDK"), "Lib"))
    add_links("vulkan-1")
