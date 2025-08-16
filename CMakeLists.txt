cmake_minimum_required(VERSION 3.16)
project(TCPServer)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Server executable
add_executable(server 
    iocp_tcp_server.cpp 
    server_main.cpp
)

# Client executable
add_executable(client 
    tcp_client.cpp 
    client_main.cpp
)

# Link Windows Socket library (ws2_32.lib)
if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif()

# Set compiler flags for Windows
if(MSVC)
    target_compile_options(server PRIVATE /EHsc)
    target_compile_options(client PRIVATE /EHsc)
endif()
