solution "sodna"
    configurations { "Debug", "Release" }

    project "sodna-sdl2"
        kind "StaticLib"
        language "C"

        links { "m", "SDL2", }

        files {
            "src/sodna_sdl2.c"
        }

        includedirs { "include/" }

        configuration { "windows" }
            includedirs { "SDL2/include" }

        configuration { "linux" }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }

    project "sodna-util"
        kind "StaticLib"
        language "C"

        links { "m", }

        includedirs { "include/" }

        files {
            "src/sodna_util.c"
        }

    project "sodna-demo"
        kind "WindowedApp"
        language "C"
        files {
            "src_demo/**.c"
        }

        includedirs {
            "include"
        }

        links {
            "m",
            "SDL2",
            "sodna-sdl2",
            "sodna-util",
        }

        configuration { "windows" }
            libdirs { "SDL2/lib-i686-w64-mingw32/" }

        configuration { "linux" }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }
