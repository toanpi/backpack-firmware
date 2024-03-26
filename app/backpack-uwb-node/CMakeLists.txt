cmake_minimum_required(VERSION 3.28)
set(CMAKE_C_STANDARD 11)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(backpack-uwb-node)
set(BUILD_TARGET anchor)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH APP_DIR)
cmake_path(GET APP_DIR PARENT_PATH BACKPACK_PROJECT_DIR)

########################################################################################
# Application Sources
########################################################################################
set(GLOB_RECURSE sources_anchor
    src/apps/anchor/*.[ch] 
    src/os/*.[ch]
    src/dw1000/*.[ch]
    )

# Todo: imrpove this
# target_link_libraries(app PRIVATE hello_world)

include_directories(include/dw1000/platform)

########################################################################################
# UWB Library
########################################################################################
set(UWB_PATH ${BACKPACK_PROJECT_DIR}/lib/uwblib)

# Soure files
file(GLOB_RECURSE UWBLIB_SOURCES 
${UWB_PATH}/application/*.[ch]
${UWB_PATH}/compiler/*.[ch]
${UWB_PATH}/device/*.[ch]
${UWB_PATH}/decadriver/*.[ch]
${UWB_PATH}/host_com/host_com_msg.c
)

# Remove location engine files
list(REMOVE_ITEM UWBLIB_SOURCES 
${UWB_PATH}/application/location_engine/location_engine.c
${UWB_PATH}/application/location_engine/location_test.c
)

# Header files
file(GLOB_RECURSE APP_ENTRIES LIST_DIRECTORIES true ${UWB_PATH})

foreach(ENTRY ${APP_ENTRIES})
    include_directories(${ENTRY})
endforeach()


########################################################################################
# Host Connection Library
########################################################################################
set(HOST_CONN_PATH ${BACKPACK_PROJECT_DIR}/lib/host_connection)

set(sources_nanopb ${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86)

file(GLOB_RECURSE HOST_CONN_SOURCES 
${HOST_CONN_PATH}/source/*.[ch]
${HOST_CONN_PATH}/projects/backpack/parser/uwb_dev/*.[ch]
${HOST_CONN_PATH}/projects/backpack/parser/backpack/*.[ch]
${HOST_CONN_PATH}/projects/backpack/proto/out_c/*.[ch]
)

file(GLOB HOST_CONN_SOURCES
${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/*.[ch]
)

# Remove file transfer files
list(REMOVE_ITEM HOST_CONN_SOURCES 
${HOST_CONN_PATH}/source/file_transfer/rgb_image_ft_file.c
${HOST_CONN_PATH}/source/file_transfer/ir_image_ft_file.c
${HOST_CONN_PATH}/source/file_transfer/hwlog_ft_file.c
)

# Header files
file(GLOB_RECURSE HC_APP_ENTRIES LIST_DIRECTORIES true ${HOST_CONN_PATH})

foreach(ENTRY ${HC_APP_ENTRIES})
    include_directories(${ENTRY})
endforeach()

########################################################################################
# Sources
########################################################################################
target_sources(app PRIVATE 
${sources_${BUILD_TARGET}}
${UWBLIB_SOURCES} 
${HOST_CONN_SOURCES}
)
