project(opencl_device_detection)
cmake_minimum_required(VERSION 3.5)

# depending on gcc version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") #  -Wall

# Flags
if(NOT WIN32)
  set(CMAKE_C_FLAGS "-Wall -pedantic -Wimplicit-function-declaration -Wreturn-type -Wstrict-prototypes -Wextra -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

# set AMD APP SDK
set(OPENCL_LIBRARY "/opt/AMDAPPSDK-3.0/lib/x86_64/sdk/libOpenCL.so")
set(OPENCL_INCLUDE_DIRECTORY "/opt/AMDAPPSDK-3.0/include")

# LINUX
if(CMAKE_COMPILER_IS_GNUCXX)
    # optimizations
    add_definitions(-O2)
endif()

include_directories(${OPENCL_INCLUDE_DIRECTORY})

file(GLOB SRC_LIST
    *.cpp )
file(GLOB HEADERS
    *.h
    *.hpp
    )

set(LIBS ${LIBS} ${OPENCL_LIBRARY})

message("-- CFLAGS: ${CMAKE_C_FLAGS}")
message("-- CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message("-- LIBS: ${LIBS}")

add_library(opencl_device_detection SHARED ${SRC_LIST} ${HEADERS})
target_link_libraries(opencl_device_detection ${LIBS})

add_library(opencl_device_detection_static STATIC ${SRC_LIST} ${HEADERS})
target_link_libraries(opencl_device_detection_static ${LIBS})

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

