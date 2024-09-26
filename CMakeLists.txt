cmake_minimum_required(VERSION 3.12)
project(webServer_template)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

file(GLOB SERVER_SRC_FILES "server.cpp")
add_executable(server ${SERVER_SRC_FILES})

file(GLOB CLIENT_SRC_FILES "client.cpp")
add_executable(client ${CLIENT_SRC_FILES})