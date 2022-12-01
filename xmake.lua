add_rules("mode.debug", "mode.release")

-- Offficial Repos
add_requires("glfw", "glm", "stb", "loguru")
add_requires("imgui v1.89-docking", { configs = {glfw_vulkan = true}})

-- Private Repos
add_repositories("myrepo myrepo")
add_requires("walnut 1.0", "lve 1.0", "emilib")

target("WalnutApp")
    set_kind("binary")
    set_languages("cxx17")
    -- set_pcxxheader("src/pch.h")
    add_files("src/*.cpp|src/pch.cpp")
    add_defines("WL_PLATFORM_WINDOWS", "WIN32")
    add_includedirs("src", path.join(os.getenv("VULKAN_SDK"), "Include"))
    add_packages("glfw", "glm", "stb", "imgui", "loguru")
    add_packages("walnut", "lve", "emilib")
    add_linkdirs(path.join(os.getenv("VULKAN_SDK"), "Lib"))
    add_links("vulkan-1")
