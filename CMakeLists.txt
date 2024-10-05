cmake_minimum_required(VERSION 3.16)
project(forward LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(PROCESS_NAME "forward")

FILE(GLOB_RECURSE FORWARD_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM FORWARD_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM FORWARD_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/sender.cpp)
list(REMOVE_ITEM FORWARD_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver.cpp)

#set(FORWARD_MANAGER_INCS ${FORWARD_SOURCE_DIR}/include)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/classes
)
    
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(FORWARD_LIBS ${FORWARD_LIBS} pthread)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "QNX")
    set(FORWARD_LIBS ${FORWARD_LIBS} socket)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG_CONFIG_ANDROID_SYSTEM=1")
    set(FORWARD_LIBS ${FORWARD_LIBS} log)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(xudp STATIC IMPORTED)
set_property(TARGET xudp PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libxudp.a)

set(FORWARD_LIBS ${FORWARD_LIBS} xudp)

add_executable(sender ${FORWARD_SRCS} src/sender.cpp)
target_link_libraries(sender PRIVATE
    ${FORWARD_LIBS}
)

add_executable(receiver ${FORWARD_SRCS} src/receiver.cpp)
target_link_libraries(receiver PRIVATE
    ${FORWARD_LIBS}
)