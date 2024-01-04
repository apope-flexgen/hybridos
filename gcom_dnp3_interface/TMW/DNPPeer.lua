-- This is an example premake file for Triangle MicroWorks, Inc.
--
-- This premake file is used to create the Visual Studio
-- solution file and the GNU Makefiles shipped with this
-- Source Code Library Example.
--  
-- To generate the Makefiles run: 
--   premake5 --file=DNPPeer.lua gmake 
--   premake5 --file=DNPPeer.lua vs2010 

-- DNPPeer.lua
workspace "DNPPeer"

   if _ACTION == "gmake" then
      configurations { "linux", "sample" }
   else
      configurations { "Debug", "Release" }
      platforms{ "Win32", "x64" }
   end

   filter "configurations:Debug"
      targetdir "%{cfg.buildcfg}"
      includedirs { "tmwscl/tmwtarg/winiotarg" }
      objdir "%{cfg.buildcfg}"
      defines { "DEBUG", "_DEBUG", "_LIB", "_CRT_SECURE_NO_WARNINGS", "TMWCNFG_INCLUDE_ASSERTS", "TMW_WTK_TARGET" }
      symbols "On"

   filter "configurations:Release"
      targetdir "%{cfg.buildcfg}"
      includedirs { "tmwscl/tmwtarg/winiotarg" }
      objdir "%{cfg.buildcfg}"
      defines { "NDEBUG", "_LIB", "_CRT_SECURE_NO_WARNINGS", "TMWCNFG_INCLUDE_ASSERTS", "TMW_WTK_TARGET" }
      optimize "On"

   filter "configurations:linux"
      targetdir "bin"
      includedirs { "tmwscl/tmwtarg/LinIoTarg" }
      defines { "TMW_LINUX_TARGET" }
      buildoptions { "-Wall", "-Wno-int-to-pointer-cast", "-W", "-g" }
      system ("linux")
      symbols "On"

   filter "configurations:sample"
      targetdir "bin"
      includedirs { "tmwscl/tmwtarg/SampleIoTarg" }
      defines { "TMW_SAMPLE_TARGET" }
      buildoptions { "-Wall", "-Wno-int-to-pointer-cast", "-W", "-g" }
      system ("linux")
      symbols "On"

   filter "platforms:Win32"
      system "Windows"
      architecture "x32"

   filter "platforms:x64"
      system "Windows"
      architecture "x64"

project "DNPPeer"
   location "DNPPeer"
   kind "ConsoleApp"
   language "C++"

   includedirs { ".", "tmwscl/tmwtarg/" }
   links { "dnp", "utils"}
   files { "DNPPeer/*.h", "DNPPeer/*.cpp" }

   filter "configurations:linux"
      links { "IoTarg", "pthread", "rt" }
      buildoptions { "-Wno-format-security" }
      removefiles "DNPPeer/StdAfx.*"

   filter "configurations:sample"
      links { "IoTarg" }
      buildoptions { "-Wno-format-security" }
      removefiles "DNPPeer/StdAfx.*"

   filter {"configurations:Debug", "platforms:Win32"}
      targetdir "./bind"
      links { "WinIoTarg", "Winmm" }
      libdirs { "./bind"}

   filter {"configurations:Debug", "platforms:x64"}
      targetdir "./bind_x64"
      links { "WinIoTarg", "Winmm" }
      libdirs { "./bind_x64"}

   filter {"configurations:Release", "platforms:Win32"}
      targetdir "./bin"
      links { "WinIoTarg", "Winmm" }
      libdirs { "./bin"}

   filter {"configurations:Release", "platforms:x64"}
      targetdir "./bin_x64"
      links { "WinIoTarg", "Winmm" }
      libdirs { "./bin_x64"}

project "dnp"
   location "tmwscl/dnp"
   kind "StaticLib"
   language "C"

   files { "tmwscl/dnp/*.h", "tmwscl/dnp/*.c" }
   includedirs { "." }

project "utils"
   location "tmwscl/utils"
   kind "StaticLib"
   language "C"


   filter "configurations:linux"
      files { "tmwscl/utils/*.h", "tmwscl/utils/*.c"}
      includedirs { "." }
      buildoptions { "-Wno-type-limits" }

   filter "configurations:sample"
      files { "tmwscl/utils/*.h", "tmwscl/utils/*.c"}
      includedirs { "." }
      buildoptions { "-Wno-type-limits" }

   filter "configurations:Debug"
      files { "tmwscl/tmwtarg/winiotarg/tmwtarg.c", "tmwscl/tmwtarg/winiotarg/tmwtargp.c", "tmwscl/tmwtarg/winiotarg/*dnptarg.c", "tmwscl/tmwtarg/winiotarg/*14targ.c", "tmwscl/utils/*.h", "tmwscl/utils/*.c"}
      includedirs { ".",  "./tmwscl/tmwtarg", "./thirdPartyCode/openssl", "./thirdPartyCode/openssl/inc32" }

   filter "configurations:Release"
      files {"tmwscl/tmwtarg/winiotarg/tmwtarg.c", "tmwscl/tmwtarg/winiotarg/tmwtargp.c", "tmwscl/tmwtarg/winiotarg/*dnptarg.c", "tmwscl/tmwtarg/winiotarg/*14targ.c" ,"tmwscl/utils/*.h", "tmwscl/utils/*.c"}
      includedirs { ".",  "./tmwscl/tmwtarg", "./thirdPartyCode/openssl", "./thirdPartyCode/openssl/inc32" }

if _ACTION == "gmake" then
   project "IoTarg"
      kind "StaticLib"
      language "C"
      location "tmwscl/tmwtarg"

      filter "configurations:linux"
         includedirs { "LinIoTarg", "." }
         files { "tmwscl/tmwtarg/LinIoTarg/*.h", "tmwscl/tmwtarg/LinIoTarg/*.c"}

      filter "configurations:sample"
         includedirs { "SampleIoTarg", "." }
         files { "tmwscl/tmwtarg/SampleIoTarg/*.h", "tmwscl/tmwtarg/SampleIoTarg/*.c"}

else
   include "tmwscl/tmwtarg/winiotarg/WinIoTarg.lua"
end

