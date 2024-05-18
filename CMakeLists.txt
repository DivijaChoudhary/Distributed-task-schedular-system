cmake_minimum_required(VERSION 3.14)
project(MultiNodeTaskScheduler)

# Set the path to your vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/Users/016915509/vcpkgnew/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the CMAKE_PREFIX_PATH to the directories where Protobuf and gRPC are installed
set(CMAKE_PREFIX_PATH "C:/Users/016915509/vcpkgnew/vcpkg/installed/x64-windows")

# Find the required packages
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(OpenMP)

# Include directories
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${gRPC_INCLUDE_DIRS})

# Proto file
set(PROTO_FILE ${CMAKE_SOURCE_DIR}/proto/scheduler.proto)

# Generate the protobuf and gRPC source files
set(PROTO_SRC_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

# Paths to the protobuf and gRPC plugins
set(PROTOC_EXECUTABLE "C:/Users/016915509/vcpkgnew/vcpkg/installed/x64-windows/tools/protobuf/protoc")
set(GRPC_CPP_PLUGIN_EXECUTABLE "C:/Users/016915509/vcpkgnew/vcpkg/installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")

# Custom command to generate the protobuf and gRPC source files
add_custom_command(
    OUTPUT ${PROTO_SRC_DIR}/scheduler.pb.cc ${PROTO_SRC_DIR}/scheduler.pb.h
           ${PROTO_SRC_DIR}/scheduler.grpc.pb.cc ${PROTO_SRC_DIR}/scheduler.grpc.pb.h
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --proto_path=${CMAKE_SOURCE_DIR}/proto --cpp_out=${PROTO_SRC_DIR} ${PROTO_FILE}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --proto_path=${CMAKE_SOURCE_DIR}/proto --grpc_out=${PROTO_SRC_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE} ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

include_directories(${PROTO_SRC_DIR})

# Add executable for scheduler server
add_executable(scheduler_server
    scheduler/scheduler_server.cpp
    scheduler/scheduler_service.cpp
    ${PROTO_SRC_DIR}/scheduler.pb.cc
    ${PROTO_SRC_DIR}/scheduler.grpc.pb.cc
)
target_link_libraries(scheduler_server gRPC::grpc++ protobuf::libprotobuf)

# Add executable for worker node
add_executable(worker_node
    worker/worker_node.cpp
    worker/task_executor.cpp
    worker/heartbeat.cpp
    worker/utils/utils.cpp
    ${PROTO_SRC_DIR}/scheduler.pb.cc
    ${PROTO_SRC_DIR}/scheduler.grpc.pb.cc
)

# Link libraries
target_link_libraries(worker_node gRPC::grpc++ protobuf::libprotobuf Threads::Threads)

# Check for OpenMP and link it if available
if(OpenMP_CXX_FOUND)
    target_link_libraries(worker_node OpenMP::OpenMP_CXX)
endif()
