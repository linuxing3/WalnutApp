package("lve")
    add_urls("https://github.com/blurrypiano/littleVulkanEngine.git")
    add_versions("1.0", "1af74f2091d1bfd241454ea46299dd6acf5638f9")
 
    add_includedirs("include")
    
    on_install("windows", function (package)
        io.writefile("xmake.lua", [[
add_requires("glfw", "glm")

target("walnut")
    set_kind("static")
    set_languages("cxx17")
    add_defines("WL_PLATFORM_WINDOWS")
    add_includedirs("src", "external/tinyobjloader", path.join(os.getenv("VULKAN_SDK"), "Include"))
    add_files("src/**.cpp")
    add_packages("glfw", "glm")
    add_linkdirs(path.join(os.getenv("VULKAN_SDK"), "Lib"))
    add_links("vulkan-1")
]])
        os.mkdir("include")
        os.cp("src/**.hpp", package:installdir(path.join("include")))
        os.cp("src/systems/*.hpp", package:installdir(path.join("include", "systems")))
        import("package.tools.xmake").install(package)
    end)
package_end()
