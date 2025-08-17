#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main() {
    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");
        asio::connect(socket, endpoints);

        std::cout << "Connected to server. Enter messages to send (type 'quit' to exit):\n";

        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "quit") {
                std::cout << "Disconnecting...\n";
                break;
            }

            if (input.empty()) continue;

            std::vector<char> data(input.begin(), input.end());
            asio::write(socket, asio::buffer(data));

            std::vector<char> buffer(1024);
            size_t len = socket.read_some(asio::buffer(buffer));
            buffer.resize(len);
            std::cout << "Server response: " << std::string(buffer.begin(), buffer.end()) << "\n";
            std::cout << "Enter next message: ";
        }

        socket.close();
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}