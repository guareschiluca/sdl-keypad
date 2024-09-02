cmake_minimum_required(VERSION 3.0)
project(build)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
file(GLOB_RECURSE SOURCES "SDL Keypad/*.cpp" "SDL Keypad/*.h")

# Add the preload file option
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${CMAKE_SOURCE_DIR}\\res@/res\"")

# Add include directories
include_directories("SDL2/include" "SDL2/TTF/include")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link SDL2, SDL2_ttf, FreeType, HarfBuzz, and zlib libraries for Emscripten
target_link_libraries(${PROJECT_NAME} SDL2 SDL2_ttf freetype harfbuzz z)
