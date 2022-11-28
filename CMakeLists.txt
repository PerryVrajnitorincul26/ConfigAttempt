cmake_minimum_required(VERSION 3.24)
project(SMG)
set(CMAKE_CXX_STANDARD 17)

#We only use local packages to avoid messing with environment variables too much
LIST(APPEND CMAKE_PREFIX_PATH vcpkg_installed)
IF (WIN32)
    INCLUDE_DIRECTORIES(vcpkg_installed/x64-windows/include)
ELSEIF (UNIX AND NOT APPLE)
    INCLUDE_DIRECTORIES(vcpkg_installed/x64-linux/include)
ENDIF ()

#NOTE: OpenGL is not available through vcpkg and must be in path.
find_package(OpenGL REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

add_executable(SMG main.cpp)

target_link_libraries(SMG PRIVATE GLEW)
target_link_libraries(SMG PRIVATE glfw)
target_link_libraries(SMG PRIVATE tinyobjloader::tinyobjloader)
target_link_libraries(SMG PRIVATE OpenGL)