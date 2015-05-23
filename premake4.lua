solution "sodna"
    configurations { "Debug", "Release" }

    project "sodna"
        kind "SharedLib"
        language "C"

        links { "m", "SDL2", }

        files {
            "src/sodna_sdl2.c",
            "src/sodna_util.c",
        }

        includedirs { "include/" }

        configuration { "windows" }
            includedirs { "SDL2/include" }

        configuration { "linux" }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }

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
            "sodna",
        }

        configuration { "windows" }
            libdirs { "SDL2/lib-i686-w64-mingw32/" }

        configuration { "linux" }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`", "-Wl,-rpath=." }
