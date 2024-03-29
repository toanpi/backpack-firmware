cmake_minimum_required(VERSION 3.28)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
set(CMAKE_C_STANDARD 23)

project(backpack-uwb-node)

########################################################################################
# Library
########################################################################################
include_directories(include) # Prerequisites
include(host_connection.cmake)
include(uwblib.cmake)

# target_link_libraries(app PRIVATE hello_world)

########################################################################################
# Application Sources
########################################################################################
set(APP_SOURCE 
src/apps/anchor/main.c
src/dw1000/dw1000_board.c
src/zephyr_os/dw_decawave.c 
src/zephyr_os/dw_os.c 
src/zephyr_os/dw_storage.c
src/zephyr_os/dw_serial.c
src/zephyr_os/dw_msg_manager.c
)

########################################################################################
# Sources
########################################################################################
target_sources(app PRIVATE 
${APP_SOURCE}
${HOST_CONN_SOURCES}
${UWBLIB_SOURCES} 
)
