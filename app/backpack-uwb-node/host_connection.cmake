########################################################################################
# Host Connection Library
########################################################################################

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH APP_DIR)
cmake_path(GET APP_DIR PARENT_PATH LIB_DIR)

set(HOST_CONN_PATH ${LIB_DIR}/lib/host_connection)

# Should be included in the project
# include_directories(include/dw1000/platform)

file(GLOB_RECURSE HOST_CONN_SOURCES 
${HOST_CONN_PATH}/source/*.[ch]
${HOST_CONN_PATH}/projects/backpack/parser/uwb_dev/*.[ch]
${HOST_CONN_PATH}/projects/backpack/parser/backpack/*.[ch]
${HOST_CONN_PATH}/projects/backpack/proto/out_c/*.[ch]
)

file(GLOB HOST_CONN_SOURCES
${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/*.[ch]
)

file(GLOB_RECURSE HC_UNUSED_SOURCES 
${HOST_CONN_PATH}/source/file_transfer/rgb_image_ft_file.c
${HOST_CONN_PATH}/source/file_transfer/ir_image_ft_file.c
${HOST_CONN_PATH}/source/file_transfer/hwlog_ft_file.c
)
# Remove file transfer files
list(REMOVE_ITEM HOST_CONN_SOURCES ${HC_UNUSED_SOURCES})

# Header files
file(GLOB_RECURSE HOST_CONN_DIRS LIST_DIRECTORIES true ${HOST_CONN_PATH})
include_directories(${HOST_CONN_DIRS})
