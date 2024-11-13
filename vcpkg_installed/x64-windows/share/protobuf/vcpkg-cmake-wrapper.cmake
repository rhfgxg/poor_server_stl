find_program(Protobuf_PROTOC_EXECUTABLE NAMES protoc PATHS "${CMAKE_CURRENT_LIST_DIR}/../../../x64-windows/tools/protobuf" NO_DEFAULT_PATH)

_find_package(${ARGS} CONFIG)
