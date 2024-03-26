cmake_minimum_required(VERSION 3.28)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(CMAKE_C_STANDARD 23)

project(backpack-uwb-node)

########################################################################################
# Library
########################################################################################
include_directories(include/dw1000/platform) # Prerequisites
include(host_connection.cmake)
include(uwblib.cmake)
# target_link_libraries(app PRIVATE hello_world)

########################################################################################
# Application Sources
########################################################################################
set(GLOB_RECURSE APP_SOURCE src)

########################################################################################
# Sources
########################################################################################
target_sources(app PRIVATE 
${APP_SOURCE}
${UWBLIB_SOURCES} 
${HOST_CONN_SOURCES}
)
