add_rules("mode.debug", "mode.release")
add_requires("glad", "glfw", "glm", "stb")
add_requires("imgui v1.89-docking", { configs = {glfw_vulkan = true}})
add_requires("walnut 1.0", "lve 1.0")

target("WalnutApp")
    set_kind("binary")
    set_languages("cxx17")
    add_defines("WL_PLATFORM_WINDOWS", "WIN32")
    add_includedirs("src", path.join(os.getenv("VULKAN_SDK"), "Include"))
    add_files("src/*.cpp")
    add_packages("glad", "glfw", "glm", "stb", "imgui", "walnut", "lve")
    add_linkdirs(path.join(os.getenv("VULKAN_SDK"), "Lib"))
    add_links("vulkan-1")
    after_build(function(target)
      for _, headerfile in ipairs(target:headerfiles()) do
         print("%s", headerfile)
      end
    end)
