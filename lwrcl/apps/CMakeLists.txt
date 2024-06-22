cmake_minimum_required(VERSION 3.16.3)
project("apps")

if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
endif()

if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

if(NOT tinyxml2_FOUND)
    find_package(tinyxml2 REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(lwrcl_example)
add_subdirectory(CustomROSTypeDataPublisher)
add_subdirectory(ROSTypeImagePubSub)
add_subdirectory(ROSTypeImagePubSubEdge)
add_subdirectory(ROSTypeImagePubSubMono)
add_subdirectory(ROSTypeImagePubSubExecutor)
add_subdirectory(CustomROSTypeDataPublisherExecutor)
add_subdirectory(ROSTypeImagePubSubExecutorTest)


