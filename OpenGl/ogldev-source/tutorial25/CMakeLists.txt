cmake_minimum_required(VERSION 3.1)
project(tutorial25)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    nbproject/private/Makefile-variables.mk
    nbproject/configurations.xml
    nbproject/Makefile-Debug.mk
    nbproject/Makefile-impl.mk
    nbproject/Makefile-variables.mk
    nbproject/Package-Debug.bash
    nbproject/Package-Release.bash
    nbproject/project.properties
    nbproject/project.xml
    lighting.fs
    lighting.vs
    lighting_technique.cpp
    lighting_technique.h
    Makefile
    mesh.cpp
    mesh.h
    skybox.cpp
    skybox.fs
    skybox.h
    skybox.vs
    skybox_technique.cpp
    skybox_technique.h
    tutorial25.cpp)

add_executable(tutorial25 ${SOURCE_FILES})