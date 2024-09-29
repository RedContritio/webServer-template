cmake_minimum_required(VERSION 3.12)
project(webServer-template)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(fmt REQUIRED)

file(GLOB SERVER_SRC_FILES "server.cpp")
add_executable(server ${SERVER_SRC_FILES})
target_link_libraries(server pthread fmt::fmt)

file(GLOB CLIENT_SRC_FILES "client.cpp")
add_executable(client ${CLIENT_SRC_FILES})

file(GLOB MULTICLIENT_SRC_FILES "multi-client.cpp")
add_executable(multi-client ${MULTICLIENT_SRC_FILES})
target_link_libraries(multi-client pthread)