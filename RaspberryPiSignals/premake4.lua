local GCC_ARM_CC_PATH = "/usr/bin/arm-linux-gnueabi-gcc"
local GCC_ARM_CPP_PATH = "/usr/bin/arm-linux-gnueabi-g++"
local GCC_ARM_AR_PATH = "/usr/bin/arm-linux-gnueabi-ar"

table.insert(premake.option.list["platform"].allowed, { "arm" })

premake.platforms.arm = {
	cfgsuffix = "arm",
	iscrosscompiler = true
}

table.insert(premake.fields.platforms.allowed, "arm")


if(_OPTIONS.platform == 'arm') then
	premake.gcc.cc = GCC_AVR_CC_PATH
	premake.gcc.cxx = GCC_AVR_CPP_PATH
	premake.gcc.ar = GCC_AVR_AR_PATH
end

solution "RaspberryPiSignals"
  configurations {"debug","release"}
  platforms {"arm", "native"}
  project "RaspberryPiSignals"
	  targetname "paper-signals"
  	language "C++"
  	kind "ConsoleApp"

		local includeFiles = {
			"**.h",
			"**.c",
			"**.cpp",
		
		}
		files(includeFiles)
		excludes({"ArduinoJson/**"})
		includedirs({"ArduinoJson/src"})
    defines { "RPI" }
    buildoptions{"-O2"," -Wformat=2", "-Wall", "-pipe", "-fPIC", "-std=c++11" }
		links {
			"pthread","rt","curl","wiringPi"
	 }

  	configuration "debug"
        defines { "DEBUG","SERIAL_DEBUG" }
				targetdir "build/debug/"
				objdir "build/debug/obj"

    configuration "release"
        defines { "NDEBUG" }
				targetdir "build/release"
				objdir "build/release/obj"
				-- postbuildcommands {
        --   "/etc/init.d/radioreceiver stop"
				-- 	"cp build/release/radioreceiver ~/RadioReceiver/"
        --   "/etc/init.d/radioreceiver start"
				-- }
