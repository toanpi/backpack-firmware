
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

# Get all entries in the application directory
file(GLOB APP_ENTRIES "application/*")

foreach(ENTRY ${APP_ENTRIES})
    if(IS_DIRECTORY ${ENTRY})
        # Include directories found inside the application directory
        include_directories(${ENTRY})
    endif()
endforeach()

include_directories("platform")

# Optionally, specify include directories for this library
include_directories("application")
include_directories("application/instance")
include_directories("application/tdma")
include_directories("application/power")
include_directories("decadriver")
include_directories("device/decawave")
include_directories("host_com")
include_directories("platform/zephyr")
