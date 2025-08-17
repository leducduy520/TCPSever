#include "asio_tcp_server.hpp"
#include <boost/asio.hpp>
#include <iostream>

int main() {
    try {
        boost::asio::io_context io_context;
        AsioTCPServer server(io_context, 8080);
        std::cout << "Server running on port 8080. Press Enter to stop...\n";
        server.start();
        std::cin.get();
        server.stop();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}