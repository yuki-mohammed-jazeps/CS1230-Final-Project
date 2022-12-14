cmake_minimum_required(VERSION 3.16)

# Sets project name
project(projects_realtime LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/lighting.cpp
    src/fullscreen.cpp
    src/texture.cpp
    src/camera.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp
    src/utils/transforms.cpp
    src/utils/obj_loader.cpp
    src/shapes/geometry.cpp
    src/shapes/cube.cpp
    src/shapes/cylinder.cpp
    src/shapes/cone.cpp
    src/shapes/sphere.cpp
    src/shapes/triangle.cpp
    src/shapes/mesh.cpp
    src/particle.cpp

    src/mainwindow.h
    src/lighting.h
    src/texture.h
    src/realtime.h
    src/fullscreen.h
    src/camera.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/transforms.h
    src/utils/obj_loader.h
    src/shapes/geometry.h
    src/shapes/cube.h
    src/shapes/cylinder.h
    src/shapes/cone.h
    src/shapes/sphere.h
    src/shapes/triangle.h
    src/shapes/mesh.h
    src/particle.h
)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c)
include_directories(${PROJECT_NAME} PRIVATE glew/include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
        resources/shaders/parallax.frag
        resources/shaders/parallax.vert
        resources/shaders/phong.frag
        resources/shaders/phong.vert
        resources/shaders/texture.frag
        resources/shaders/texture.vert
        resources/shaders/shadow.frag
        resources/shaders/shadow.vert
        resources/shaders/particle.vert
        resources/shaders/particle.frag
        resources/scene/Particle.png
)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
