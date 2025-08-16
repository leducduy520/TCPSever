#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

class TCPClient {
public:
    TCPClient(const std::string& serverIP, unsigned short port);
    ~TCPClient();

    void sendData(const std::vector<char>& data);
    std::vector<char> receiveData(size_t bufferSize = 1024);

private:
    SOCKET socket_ = INVALID_SOCKET;
    WSADATA wsaData_;
};