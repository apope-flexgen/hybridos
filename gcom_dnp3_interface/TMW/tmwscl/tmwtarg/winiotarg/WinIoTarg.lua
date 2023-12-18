-- WinIoTarg.lua

project "winiotarg"
   location "."
   kind "SharedLib"
   language "C++"
   objdir "%{cfg.buildcfg}"
      
   defines { "WINIOTARG_EXPORTS", "_AFXDLL", "_WINDOWS" }
   removedefines "TMWCNFG_INCLUDE_ASSERTS"
   files { "*.h", "*.cpp" }

   filter {"configurations:Debug", "platforms:Win32"}
      includedirs { "..", "../../../", "../../../thirdPartyCode/openssl", "../../../thirdPartyCode/openssl/inc32" }
      targetdir "../../../bind"
	  libdirs { "../../../bind", "../../../thirdPartyCode/openssl/out32dll.dbg" }

   filter {"configurations:Debug", "platforms:x64"}
      includedirs { "..", "../../../", "../../../thirdPartyCode/openssl", "../../../thirdPartyCode/openssl/inc64" }
      targetdir "../../../bind_x64"
	  libdirs { "../../../bind_x64", "../../../thirdPartyCode/openssl/out64dll.dbg" }

   filter {"configurations:Release", "platforms:Win32"}
      includedirs { "..", "../../../", "../../../thirdPartyCode/openssl", "../../../thirdPartyCode/openssl/inc32" }
      targetdir "../../../bin"
	  libdirs { "../../../bin", "../../../thirdPartyCode/openssl/out32dll" }

   filter {"configurations:Release", "platforms:x64"}
      includedirs { "..", "../../../", "../../../thirdPartyCode/openssl", "../../../thirdPartyCode/openssl/inc64" }
      targetdir "../../../bin_x64"
	  libdirs { "../../../bin_x64", "../../../thirdPartyCode/openssl/out64dll" }
