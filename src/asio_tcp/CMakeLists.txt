project(AsioTCPServer)

# Find Boost
find_package(Boost REQUIRED COMPONENTS asio)

add_library(asio_tcp_server SHARED asio_tcp_server.cpp)
target_include_directories(asio_tcp_server PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(asio_tcp_server PROPERTIES OUTPUT_NAME "libAsioTcpServer" WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Link Windows Socket library (ws2_32.lib)
if(WIN32)
    target_link_libraries(asio_tcp_server ws2_32 Boost::asio)
endif()

# Set compiler flags for Windows
if(MSVC)
    target_compile_options(asio_tcp_server PRIVATE /EHsc)
endif()
