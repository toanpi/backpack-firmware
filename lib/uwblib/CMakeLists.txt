cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Set the project name
project(uwblib)

# Use GLOB_RECURSE to find all .c and .h files in the specified directory and subdirectories

file(GLOB_RECURSE UWBLIB_SOURCES 
"compiler/*.[ch]"
"device/*.[ch]"
"application/app/*.[ch]"
"application/exchange/*.[ch]"
"application/power/*.[ch]"
"application/instance/*.[ch]"
"application/tdma/*.[ch]"
"application/debug/*.[ch]"
"application/dev_config/*.[ch]"
# "application/location_engine/*.[ch]"
"application/twr/*.[ch]"
"application/distance/*.[ch]"
"application/uwb_dev/*.[ch]"
"application/event/*.[ch]"
"application/network/*.[ch]"
"application/test/*.[ch]"
"application/lib.h"
"application/application_definitions.h"
"decadriver/*.[ch]"
"host_com/host_com_msg.c"
)

# file(GLOB_RECURSE UWBLIB_SOURCES "decadriver/*.[ch]")

# Add a library target with the collected source and header files
# add_library(uwblib SHARED ${UWBLIB_SOURCES})
add_library(uwblib STATIC ${UWBLIB_SOURCES})


target_link_libraries(uwblib PRIVATE host_connection)

# Get all entries in the application directory
file(GLOB APP_ENTRIES "application/*")

foreach(ENTRY ${APP_ENTRIES})
    if(IS_DIRECTORY ${ENTRY})
        # Include directories found inside the application directory
        include_directories(${ENTRY})
    endif()
endforeach()

include_directories(uwblib STATIC "platform")


# Optionally, specify include directories for this library
target_include_directories(uwblib PUBLIC "application")
target_include_directories(uwblib PUBLIC "application/instance")
target_include_directories(uwblib PUBLIC "application/tdma")
target_include_directories(uwblib PUBLIC "application/power")
target_include_directories(uwblib PUBLIC "decadriver")
target_include_directories(uwblib PUBLIC "device/decawave")
target_include_directories(uwblib PUBLIC "host_com")
target_include_directories(uwblib PUBLIC "platform/zephyr")

# Set the floating-point ABI for the entire project
# add_compile_options(-mfloat-abi=hard)

# Alternatively, set it for a specific target
target_compile_options(uwblib PRIVATE -mfloat-abi=hard -mfpu=fpv4-sp-d16)

