########################################################################################
# UWB Library
########################################################################################
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH APP_DIR)
cmake_path(GET APP_DIR PARENT_PATH LIB_DIR)

set(UWB_PATH ${LIB_DIR}/lib/uwblib)

# Should be included in the project
# include_directories(include/dw1000/platform)

# Soure files
file(GLOB_RECURSE UWBLIB_SOURCES 
${UWB_PATH}/application/*.[ch]
${UWB_PATH}/compiler/*.[ch]
${UWB_PATH}/device/*.[ch]
${UWB_PATH}/decadriver/*.[ch]
${UWB_PATH}/host_com/host_com_msg.c
)

# Remove location engine files
file(GLOB_RECURSE UWBLIB_UNUSED_SOURCES 
${UWB_PATH}/application/location_engine/*.[ch]
)
list(REMOVE_ITEM UWBLIB_SOURCES ${UWBLIB_UNUSED_SOURCES})

# Header files
file(GLOB_RECURSE APP_ENTRIES LIST_DIRECTORIES true ${UWB_PATH})

include_directories(${APP_ENTRIES})
