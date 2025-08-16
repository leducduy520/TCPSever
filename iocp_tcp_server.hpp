#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

struct PER_IO_DATA {
    OVERLAPPED overlapped = {0};
    WSABUF wsabuf;
    char buffer[1024];
    DWORD bytesTransferred = 0;
    SOCKET clientSocket = INVALID_SOCKET;
    bool isRecv = true;
};

class IOCPTCPServer {
public:
    IOCPTCPServer(unsigned short port, int backlog = SOMAXCONN);
    ~IOCPTCPServer();

    void start(int numThreads = 0);
    void stop();

private:
    void workerThread();
    void acceptConnections();
    void processIO(PER_IO_DATA* ioData, DWORD bytesTransferred);

    SOCKET listenSocket_ = INVALID_SOCKET;
    HANDLE iocpHandle_ = INVALID_HANDLE_VALUE;
    std::vector<std::thread> workerThreads_;
    std::atomic<bool> running_ = false;
    WSADATA wsaData_;
    unsigned short port_;
    int backlog_;
};