#include <string>
#include "asio_tcp_server.hpp"

AsioTCPServer::AsioTCPServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), port_(port) {
    std::cout << "Server initialized on port " << port << "\n";
}

AsioTCPServer::~AsioTCPServer() {
    stop();
}

void AsioTCPServer::start(int numThreads) {
    if (running_) return;
    running_ = true;

    if (numThreads == 0) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numThreads = sysInfo.dwNumberOfProcessors * 2;
    }

    acceptConnections();
    for (int i = 0; i < numThreads - 1; ++i) { // -1 because main thread also runs io_context
        workerThreads_.emplace_back([&]() { io_context_.run(); });
    }
    io_context_.run(); // Main thread also processes I/O
}

void AsioTCPServer::stop() {
    if (!running_) return;
    running_ = false;
    acceptor_.close();
    io_context_.stop();
    for (auto& thread : workerThreads_) {
        if (thread.joinable()) thread.join();
    }
    workerThreads_.clear();
}

void AsioTCPServer::acceptConnections() {
    auto socket = std::make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        handleAccept(socket, error);
        if (running_) acceptConnections(); // Accept next connection
    });
}

void AsioTCPServer::handleAccept(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
    if (!error) {
        std::cout << "Client connected.\n";
        readFromClient(socket);
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
}

void AsioTCPServer::readFromClient(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024); // Allocate buffer
    socket->async_read_some(asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code& error, size_t bytesTransferred) {
        handleRead(socket, error, bytesTransferred, buffer); // Pass buffer to handleRead
    });
}

void AsioTCPServer::handleRead(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<std::vector<char>> buffer) {
    if (!error) {
        std::string received(buffer->data(), bytesTransferred);
        std::cout << "Received: " << received << "\n";

        // Echo back
        asio::async_write(*socket, asio::buffer(*buffer, bytesTransferred), [this, socket, buffer](const boost::system::error_code& error, size_t bytesTransferred) {
            handleWrite(socket, error, bytesTransferred, buffer); // Pass buffer to handleWrite
        });
    } else if (error != asio::error::eof) {
        std::cerr << "Read error: " << error.message() << "\n";
    } else {
        std::cout << "Client disconnected.\n";
        socket->close();
    }
}

void AsioTCPServer::handleWrite(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<std::vector<char>> buffer) {
    if (!error) {
        readFromClient(socket); // Prepare for next read
    } else {
        std::cerr << "Write error: " << error.message() << "\n";
        socket->close();
    }
}