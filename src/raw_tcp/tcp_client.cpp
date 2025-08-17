#include "tcp_client.hpp"

TCPClient::TCPClient(const std::string& serverIP, unsigned short port) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(WSAGetLastError()));
    }

    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed: " + std::to_string(WSAGetLastError()));
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (connect(socket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(socket_);
        WSACleanup();
        throw std::runtime_error("Connection failed: " + std::to_string(WSAGetLastError()));
    }
}

TCPClient::~TCPClient() {
    closesocket(socket_);
    WSACleanup();
}

void TCPClient::sendData(const std::vector<char>& data) {
    int bytesSent = send(socket_, data.data(), static_cast<int>(data.size()), 0);
    if (bytesSent == SOCKET_ERROR) {
        throw std::runtime_error("Send failed: " + std::to_string(WSAGetLastError()));
    }
}

std::vector<char> TCPClient::receiveData(size_t bufferSize) {
    std::vector<char> buffer(bufferSize);
    int bytesReceived = recv(socket_, buffer.data(), static_cast<int>(bufferSize), 0);
    if (bytesReceived == SOCKET_ERROR) {
        throw std::runtime_error("Receive failed: " + std::to_string(WSAGetLastError()));
    } else if (bytesReceived == 0) {
        throw std::runtime_error("Connection closed by server");
    }
    buffer.resize(bytesReceived);
    return buffer;
}