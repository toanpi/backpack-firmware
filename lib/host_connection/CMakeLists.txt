cmake_minimum_required(VERSION 3.20)

# Set the project name
project(host_connection)

# Use GLOB_RECURSE to find all .c and .h files in the specified directory and subdirectories

file(GLOB_RECURSE HOST_CONN_SOURCES 
"source/*.[ch]"
"projects/backpack/proto/out_c/*.[ch]"
)


set(sources_nanopb "third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86")

file(GLOB HOST_CONN_SOURCES "third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/*.[ch]")


# Add a library target with the collected source and header files
add_library(host_connection STATIC ${HOST_CONN_SOURCES})

# Get all entries in the application directory
file(GLOB SRC_ENTRIES "source/*")

foreach(ENTRY ${SRC_ENTRIES})
    if(IS_DIRECTORY ${ENTRY})
        # Include directories found inside the application directory
        include_directories(${ENTRY})
    endif()
endforeach()
        
target_include_directories(host_connection PUBLIC "third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86")

# Optionally, specify include directories for this library
target_include_directories(host_connection PUBLIC "source/host_connection")
target_include_directories(host_connection PUBLIC "source/packet")
target_include_directories(host_connection PUBLIC "projects/backpack/parser/uwb_dev")
target_include_directories(host_connection PUBLIC "projects/backpack/parser/backpack")
target_include_directories(host_connection PUBLIC "projects/backpack/proto/out_c")

