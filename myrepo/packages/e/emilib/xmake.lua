package("emilib")
  add_urls("https://github.com/emilk/emilib.git")
  add_versions("1.0","0d90f4c0cd44813f6898b417bf3f4ef574a5b136")
  
  add_configs("header_only", {description = "Use header only version.", default = true, type = "boolean"})

  on_load(function (package)
    if package:config("header_only") then
        package:add("defines", "EMILIB_HEADER_ONLY=1")
    end
    if package:config("shared") then
        package:add("defines", "FMT_EXPORT")
    end
  end)
  
  on_install(function(package)
    if package:config("header_only") then
      os.cp("emilib", package:installdir("include"))
      return
    end
  end)