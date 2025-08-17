project(RawTCPServer)

# Raw TCP Server library
add_library(raw_tcp_server SHARED iocp_tcp_server.cpp)
target_include_directories(raw_tcp_server PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(raw_tcp_server PROPERTIES OUTPUT_NAME "libRawTcpServer" WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Raw TCP Client library
add_library(raw_tcp_client SHARED tcp_client.cpp)
target_include_directories(raw_tcp_client PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(raw_tcp_client PROPERTIES OUTPUT_NAME "libTcpClient" WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Link Windows Socket library (ws2_32.lib)
if(WIN32)
    target_link_libraries(raw_tcp_server ws2_32)
    target_link_libraries(raw_tcp_client ws2_32)
endif()

# Set compiler flags for Windows
if(MSVC)
    target_compile_options(raw_tcp_server PRIVATE /EHsc)
    target_compile_options(raw_tcp_client PRIVATE /EHsc)
endif()
