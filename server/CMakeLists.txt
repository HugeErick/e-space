cmake_minimum_required(VERSION 3.15)
project(hello_grpc)

set(CMAKE_CXX_STANDARD 17)

# Find packages - use CONFIG mode for both
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# ---- generated sources ----
set(PROTO e-space.proto)
set(PROTO_SRCS   ${CMAKE_CURRENT_BINARY_DIR}/e-space.pb.cc)
set(PROTO_HDRS   ${CMAKE_CURRENT_BINARY_DIR}/e-space.pb.h)
set(GRPC_SRCS    ${CMAKE_CURRENT_BINARY_DIR}/e-space.grpc.pb.cc)
set(GRPC_HDRS    ${CMAKE_CURRENT_BINARY_DIR}/e-space.grpc.pb.h)

# Get the protoc compiler location
get_target_property(_PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT _PROTOBUF_PROTOC)
    get_target_property(_PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_DEBUG)
endif()
if(NOT _PROTOBUF_PROTOC)
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

add_custom_command(
  OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
  COMMAND ${_PROTOBUF_PROTOC}
    -I "${CMAKE_CURRENT_SOURCE_DIR}"
    --cpp_out   "${CMAKE_CURRENT_BINARY_DIR}"
    --grpc_out  "${CMAKE_CURRENT_BINARY_DIR}"
    --plugin=protoc-gen-grpc="$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}"
  DEPENDS "${PROTO}")

add_library(proto_objs OBJECT ${PROTO_SRCS} ${GRPC_SRCS})
target_include_directories(proto_objs PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto_objs PUBLIC protobuf::libprotobuf gRPC::grpc++)

# ---- server executable ----
add_executable(server server.cpp $<TARGET_OBJECTS:proto_objs>)
target_link_libraries(server proto_objs gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf)
