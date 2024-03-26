########################################################################################
# UWB Library
########################################################################################
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH APP_DIR)
cmake_path(GET APP_DIR PARENT_PATH LIB_DIR)

set(UWB_PATH ${LIB_DIR}/lib/uwblib)
set(UWB_APP_PATH ${UWB_PATH}/application)

# Should be included in the project
# include_directories(include/dw1000/platform)

set(UWBLIB_SOURCES 
${UWB_APP_PATH}/app/app_monitor.c
${UWB_APP_PATH}/app/app_state.c
${UWB_APP_PATH}/app/app_main.c
${UWB_APP_PATH}/app/timestamp.c
${UWB_APP_PATH}/debug/system_dbg.c
${UWB_APP_PATH}/dev_config/instance_config.c
${UWB_APP_PATH}/distance/distance.c
${UWB_APP_PATH}/event/uwb_event.c
${UWB_APP_PATH}/exchange/host_msg.c
${UWB_APP_PATH}/instance/instance.c
${UWB_APP_PATH}/instance/instance_utilities.c
${UWB_APP_PATH}/main/dw_main.c
${UWB_APP_PATH}/network/discovery.c
${UWB_APP_PATH}/network/net_node.c
${UWB_APP_PATH}/network/network.c
${UWB_APP_PATH}/network/network_mac.c
${UWB_APP_PATH}/network/network_packet.c
${UWB_APP_PATH}/power/tx_power.c
${UWB_APP_PATH}/tdma/tdma_handler.c
${UWB_APP_PATH}/test/dev_test.c
${UWB_APP_PATH}/test/ranging_test.c
${UWB_APP_PATH}/twr/twr_main.c
${UWB_APP_PATH}/twr/twr_send.c
${UWB_APP_PATH}/uwb_dev/instance_calib.c
${UWB_APP_PATH}/uwb_dev/uwb_dev_driver.c
${UWB_APP_PATH}/uwb_dev/uwb_dev_isr.c
${UWB_APP_PATH}/uwb_dev/uwb_dev_setting.c
${UWB_APP_PATH}/uwb_dev/uwb_transfer.c
${UWB_PATH}/decadriver/deca_device.c
${UWB_PATH}/decadriver/deca_params_init.c
${UWB_PATH}/device/decawave/decawave_driver.c
${UWB_PATH}/device/dw1000/dw1000_driver.c
${UWB_PATH}/device/uwb_config.c
${UWB_PATH}/host_com/host_com_msg.c
)

# Header files
include_directories(
${UWB_PATH}/compiler
${UWB_PATH}/device
${UWB_PATH}/device/decawave
${UWB_PATH}/decadriver
${UWB_PATH}/host_com
${UWB_PATH}/platform
${UWB_APP_PATH}/
${UWB_APP_PATH}/app
${UWB_APP_PATH}/instance
${UWB_APP_PATH}/network
${UWB_APP_PATH}/tdma
${UWB_APP_PATH}/uwb_dev
${UWB_APP_PATH}/test
${UWB_APP_PATH}/debug
${UWB_APP_PATH}/dev_config
${UWB_APP_PATH}/distance
${UWB_APP_PATH}/event
${UWB_APP_PATH}/exchange
${UWB_APP_PATH}/main
${UWB_APP_PATH}/power
${UWB_APP_PATH}/twr
)
