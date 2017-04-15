project(wavefile)

cmake_minimum_required(VERSION 3.6)

if (NOT WIN32)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif ()

file (GLOB SRCS src/*.cpp)

add_executable (${CMAKE_PROJECT_NAME} ${SRCS})
