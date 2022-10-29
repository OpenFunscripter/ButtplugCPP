project(ButtplugCppClient)

if(MSVC)
  # Static runtime
  set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/protobuf/cmake/")
include(protobuf-generate)


add_library(bp-proto OBJECT "${CMAKE_SOURCE_DIR}/lib/buttplug-rs-ffi/protobuf_schemas/buttplug_rs_ffi.proto")
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(bp-proto PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
target_link_libraries(bp-proto PUBLIC libprotobuf)

add_library(${PROJECT_NAME} STATIC
    "ButtplugFFI.cpp"
    "ButtplugClient.cpp"
    "ButtplugProto.cpp"
    "ButtplugDevice.cpp"
)

protobuf_generate(
    LANGUAGE cpp
    TARGET bp-proto
    IMPORT_DIRS "${CMAKE_SOURCE_DIR}/lib/buttplug-rs-ffi/protobuf_schemas/"
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE dylib)
target_link_libraries(${PROJECT_NAME} PRIVATE bp-proto)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W2 /WX)
endif()

if(UNIX AND NOT APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()