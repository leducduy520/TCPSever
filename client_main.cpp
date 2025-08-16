#include "tcp_client.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

int main() {
    try {
        std::string serverIP = "127.0.0.1";  // Change to your server's IP if remote
        unsigned short port = 8080;

        TCPClient client(serverIP, port);
        std::cout << "Connected to server. Enter messages to send (type 'quit' to exit):\n";

        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "quit") {
                std::cout << "Disconnecting...\n";
                break;
            }

            if (input.empty()) continue;  // Skip empty lines

            std::vector<char> data(input.begin(), input.end());
            client.sendData(data);

            auto response = client.receiveData();
            std::cout << "Server response: " << std::string(response.begin(), response.end()) << "\n";
            std::cout << "Enter next message: ";
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}