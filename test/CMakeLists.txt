project(test)

get_target_property(include_dirs asio_tcp_server INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Include directories for asio_tcp_server: ${include_dirs}")
get_target_property(include_dirs raw_tcp_server INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Include directories for raw_tcp_server: ${include_dirs}")

add_executable(raw_server raw_server.cpp)
target_link_libraries(raw_server PRIVATE raw_tcp_server)

add_executable(asio_server asio_server.cpp)
target_link_libraries(asio_server PRIVATE asio_tcp_server)

add_executable(raw_client raw_client.cpp)
target_link_libraries(raw_client PRIVATE raw_tcp_client)

find_package(Boost REQUIRED COMPONENTS asio)
add_executable(asio_client asio_client.cpp)
target_link_libraries(asio_client PRIVATE Boost::asio)