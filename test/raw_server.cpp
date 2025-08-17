#include "iocp_tcp_server.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        IOCPTCPServer server(8080);
        std::thread serverThread([&server]() {
            server.start();
        });

        std::cout << "Server running on port 8080. Press Enter to stop...\n";
        std::cin.get();

        server.stop();
        serverThread.join();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}