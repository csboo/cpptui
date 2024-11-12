cmake_minimum_required(VERSION 3.10)
project(cpptui LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

# Get a list of all the source files in the `examples` directory
file(GLOB SOURCES "examples/*.cpp")

# Loop through each source file and create an executable with a different name
foreach(source_file ${SOURCES})
  # Get the name of the file without the extension
  get_filename_component(name ${source_file} NAME_WE)

  # Add an executable with a unique name
  add_executable(${name} ${source_file})
endforeach()


# if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
#   target_compile_options(cpptui PRIVATE -Wall -Wextra -Wunreachable-code -Wpedantic)
# endif()
# if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#   target_compile_options(cpptui PRIVATE -Wweak-vtables -Wexit-time-destructors -Wglobal-constructors -Wmissing-noreturn)
# endif()
# if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#   target_compile_options(cpptui PRIVATE /W4 /w44265 /w44061 /w44062)
# endif()
