########################################################################################
# Host Connection Library
########################################################################################

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH APP_DIR)
cmake_path(GET APP_DIR PARENT_PATH LIB_DIR)

set(HOST_CONN_PATH ${LIB_DIR}/lib/host_connection)
set(HOST_CONN_PROTO_PATH ${LIB_DIR}/lib/host_connection/projects/backpack/proto/out_c)
set(HOST_CONN_PARSE_PATH ${LIB_DIR}/lib/host_connection/projects/backpack/parser)

# Source files
set(HOST_CONN_SOURCES
${HOST_CONN_PATH}/projects/backpack/parser/backpack/backpack_parser.c
${HOST_CONN_PATH}/projects/backpack/parser/uwb_dev/uwb_dev_parser.c
${HOST_CONN_PROTO_PATH}/algorithm.pb.c
${HOST_CONN_PROTO_PATH}/backpack-config.pb.c
${HOST_CONN_PROTO_PATH}/backpack.pb.c
${HOST_CONN_PROTO_PATH}/ble-central.pb.c
${HOST_CONN_PROTO_PATH}/collector.pb.c
${HOST_CONN_PROTO_PATH}/command.pb.c
${HOST_CONN_PROTO_PATH}/file-transfer.pb.c
${HOST_CONN_PROTO_PATH}/hw_logger.pb.c
${HOST_CONN_PROTO_PATH}/log.pb.c
${HOST_CONN_PROTO_PATH}/navigation.pb.c
${HOST_CONN_PROTO_PATH}/sensor.pb.c
${HOST_CONN_PROTO_PATH}/stimulation.pb.c
${HOST_CONN_PROTO_PATH}/uwb-anchor.pb.c
${HOST_CONN_PROTO_PATH}/uwb-dev-config.pb.c
${HOST_CONN_PROTO_PATH}/uwb-dev.pb.c
${HOST_CONN_PATH}/source/file_transfer/file_transfer.c
${HOST_CONN_PATH}/source/host_connection/host_connection.c
${HOST_CONN_PATH}/source/packet/packet.c
${HOST_CONN_PATH}/source/proto_utilities/proto_utilities.c
${HOST_CONN_PATH}/source/utils/crc32.c
${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/pb_common.c
${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/pb_decode.c
${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86/pb_encode.c
)

# Include directories
include_directories(
${HOST_CONN_PARSE_PATH}/backpack
${HOST_CONN_PARSE_PATH}/uwb_dev
${HOST_CONN_PROTO_PATH}
${HOST_CONN_PATH}/source
${HOST_CONN_PATH}/source/file_transfer
${HOST_CONN_PATH}/source/host_connection
${HOST_CONN_PATH}/source/packet
${HOST_CONN_PATH}/source/proto_utilities
${HOST_CONN_PATH}/source/utils
${HOST_CONN_PATH}/projects/backpack/port
${HOST_CONN_PATH}/third_parties/protobuf/nanopb/nanopb-0.4.7-macosx-x86
)

