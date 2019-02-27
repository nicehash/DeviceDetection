project(cuda_device_detection)
cmake_minimum_required(VERSION 3.5)

option(ENABLE_CUDA "Enable the cuda build" ON)

# depending on gcc version
# ;-std=c++11 => Ubuntu 14.04 check gcc versions
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-std=c++11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") #  -Wall

# Flags
if(NOT WIN32)
  set(CMAKE_C_FLAGS "-Wall -pedantic -Wimplicit-function-declaration -Wreturn-type -Wstrict-prototypes -Wextra -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

# LINUX
if(CMAKE_COMPILER_IS_GNUCXX)
    # optimizations
    add_definitions(-O2)
endif()

# CUDA
FIND_PACKAGE(CUDA REQUIRED)
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_50,code=sm_50;)
if(CUDA_FOUND)
message("CUDA FOUND")
else()
message("CUDA NOT FOUND")
endif()

file(GLOB SRC_LIST
    *.cpp )
file(GLOB HEADERS
    *.h
    )

# NVIDIA driver nvml stubs
link_directories(/usr/local/cuda/lib64/stubs/)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CUDA_INCLUDE_DIRS})

set(LIBS ${LIBS} ${CUDA_LIBRARIES})
set(LIBS ${LIBS} nvidia-ml)


message("-- CFLAGS: ${CMAKE_C_FLAGS}")
message("-- CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message("-- LIBS: ${LIBS}")

cuda_add_library(cuda_device_detection SHARED ${SRC_LIST} ${HEADERS})
target_link_libraries(cuda_device_detection ${LIBS})

cuda_add_library(cuda_device_detection_static STATIC ${SRC_LIST} ${HEADERS})
target_link_libraries(cuda_device_detection_static ${LIBS})

CUDA_ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${LIBS})
