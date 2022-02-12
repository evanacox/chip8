cmake_minimum_required(VERSION 3.20)
project(chip8)

set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

add_executable(chip8 src/main.cc src/driver.cc src/debug.h src/debug.cc src/vm.cc src/display.h src/display.cc)
target_compile_features(chip8 PRIVATE cxx_std_20)

target_link_libraries(chip8 PRIVATE sfml-system sfml-window sfml-window sfml-audio)

if (${MSVC})
    target_compile_options(chip8 PRIVATE /W3)
else ()
    target_compile_options(chip8 PRIVATE
            -Wall
            -Wextra
            -Wcast-qual
            -Wconversion-null
            -Wmissing-declarations
            -Woverlength-strings
            -Wpointer-arith
            -Wunused-local-typedefs
            -Wunused-result
            -Wvarargs
            -Wvla
            -Wwrite-strings
            -Werror
            -ftemplate-backtrace-limit=20)

    if (SHOULD_SANITIZE)
        target_compile_options(chip8 PRIVATE -fsanitize=undefined,address)
        target_link_options(chip8 PRIVATE -fsanitize=undefined,address)
    endif ()
endif ()