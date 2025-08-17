#include <string>
#include "iocp_tcp_server.hpp"

IOCPTCPServer::IOCPTCPServer(unsigned short port, int backlog) : port_(port), backlog_(backlog) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(WSAGetLastError()));
    }

    listenSocket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket_ == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed: " + std::to_string(WSAGetLastError()));
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);

    if (bind(listenSocket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket_);
        WSACleanup();
        throw std::runtime_error("Bind failed: " + std::to_string(WSAGetLastError()));
    }

    if (listen(listenSocket_, backlog_) == SOCKET_ERROR) {
        closesocket(listenSocket_);
        WSACleanup();
        throw std::runtime_error("Listen failed: " + std::to_string(WSAGetLastError()));
    }

    iocpHandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (iocpHandle_ == nullptr) {
        closesocket(listenSocket_);
        WSACleanup();
        throw std::runtime_error("CreateIoCompletionPort failed: " + std::to_string(GetLastError()));
    }

    if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket_), iocpHandle_, 0, 0) == nullptr) {
        closesocket(listenSocket_);
        CloseHandle(iocpHandle_);
        WSACleanup();
        throw std::runtime_error("Associate listen socket with IOCP failed: " + std::to_string(GetLastError()));
    }
}

IOCPTCPServer::~IOCPTCPServer() {
    stop();
    closesocket(listenSocket_);
    CloseHandle(iocpHandle_);
    WSACleanup();
}

void IOCPTCPServer::start(int numThreads) {
    if (running_) return;
    running_ = true;

    if (numThreads == 0) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numThreads = sysInfo.dwNumberOfProcessors * 2;
    }

    for (int i = 0; i < numThreads; ++i) {
        workerThreads_.emplace_back(&IOCPTCPServer::workerThread, this);
    }

    acceptConnections();
}

void IOCPTCPServer::stop() {
    if (!running_) return;
    running_ = false;

    for (size_t i = 0; i < workerThreads_.size(); ++i) {
        PostQueuedCompletionStatus(iocpHandle_, 0, 0, nullptr);
    }

    for (auto& thread : workerThreads_) {
        if (thread.joinable()) thread.join();
    }
    workerThreads_.clear();
}

void IOCPTCPServer::acceptConnections() {
    while (running_) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = WSAAccept(listenSocket_, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen, nullptr, 0);
        if (clientSocket == INVALID_SOCKET) {
            if (running_) {
                std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
            }
            continue;
        }

        if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), iocpHandle_, 0, 0) == nullptr) {
            std::cerr << "Associate client socket with IOCP failed: " << GetLastError() << "\n";
            closesocket(clientSocket);
            continue;
        }

        PER_IO_DATA* ioData = new PER_IO_DATA();
        ioData->clientSocket = clientSocket;
        ioData->wsabuf.buf = ioData->buffer;
        ioData->wsabuf.len = sizeof(ioData->buffer);
        ioData->isRecv = true;

        DWORD flags = 0;
        if (WSARecv(clientSocket, &ioData->wsabuf, 1, nullptr, &flags, &ioData->overlapped, nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                std::cerr << "WSARecv failed: " << WSAGetLastError() << "\n";
                closesocket(clientSocket);
                delete ioData;
                continue;
            }
        }
        std::cout << "Client connected.\n";
    }
}

void IOCPTCPServer::workerThread() {
    while (running_) {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OVERLAPPED* overlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(iocpHandle_, &bytesTransferred, &completionKey, &overlapped, INFINITE);
        if (!result) {
            if (GetLastError() == WAIT_TIMEOUT) continue;
            if (overlapped == nullptr) break;
            std::cerr << "GetQueuedCompletionStatus failed: " << GetLastError() << "\n";
            continue;
        }

        if (bytesTransferred == 0 && overlapped == nullptr) break;

        PER_IO_DATA* ioData = reinterpret_cast<PER_IO_DATA*>(overlapped);
        processIO(ioData, bytesTransferred);
    }
}

void IOCPTCPServer::processIO(PER_IO_DATA* ioData, DWORD bytesTransferred) {
    if (bytesTransferred == 0) {
        closesocket(ioData->clientSocket);
        delete ioData;
        std::cout << "Client disconnected.\n";
        return;
    }

    if (ioData->isRecv) {
        std::string received(ioData->buffer, bytesTransferred);
        std::cout << "Received from client: " << received << "\n";

        ioData->wsabuf.len = bytesTransferred;
        memcpy(ioData->buffer, received.c_str(), bytesTransferred);
        ioData->isRecv = false;

        if (WSASend(ioData->clientSocket, &ioData->wsabuf, 1, nullptr, 0, &ioData->overlapped, nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                std::cerr << "WSASend failed: " << WSAGetLastError() << "\n";
                closesocket(ioData->clientSocket);
                delete ioData;
                return;
            }
        }
    } else {
        ioData->wsabuf.len = sizeof(ioData->buffer);
        ioData->isRecv = true;
        memset(&ioData->overlapped, 0, sizeof(OVERLAPPED));

        DWORD flags = 0;
        if (WSARecv(ioData->clientSocket, &ioData->wsabuf, 1, nullptr, &flags, &ioData->overlapped, nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                std::cerr << "WSARecv failed: " << WSAGetLastError() << "\n";
                closesocket(ioData->clientSocket);
                delete ioData;
                return;
            }
        }
    }
}