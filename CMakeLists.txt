cmake_minimum_required(VERSION 3.23)
project(SMG)
set(CMAKE_CXX_STANDARD 17)

#We only use local packages to avoid messing with environment variables too much
IF (WIN32)
    INCLUDE_DIRECTORIES(vcpkg_installed/x64-windows/include)
    INCLUDE_DIRECTORIES(vcpkg_installed/x86-windows/include)
    LIST(APPEND CMAKE_PREFIX_PATH vcpkg_installed/x64-windows)
ELSEIF (UNIX AND NOT APPLE)
    INCLUDE_DIRECTORIES(vcpkg_installed/x64-linux/include)
    LIST(APPEND CMAKE_PREFIX_PATH vcpkg_installed/x64-linux)
ENDIF ()

#NOTE: OpenGL is not available through vcpkg and must be in path.
find_package(OpenGL REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW  REQUIRED)

IF (UNIX AND NOT APPLE)
    set(GLEW_LIBRARIES GLEW)
ENDIF ()
find_package(imgui CONFIG REQUIRED COMPONENTS core glfw)

set(ALL_LIBS ${GLEW_LIBRARIES} glfw tinyobjloader::tinyobjloader ${OPENGL_LIBRARIES} imgui::imgui)

add_executable(SMG main.cpp Camera.h Camera.cpp)
target_link_libraries(SMG PRIVATE ${ALL_LIBS})

add_executable(imguitest projimgui.cpp)
target_link_libraries(imguitest PRIVATE ${ALL_LIBS})

add_executable(tinTest tinyobjTest.h)
target_link_libraries(tinTest PRIVATE ${ALL_LIBS})

file(COPY shaders DESTINATION .)
file(COPY models DESTINATION .)

