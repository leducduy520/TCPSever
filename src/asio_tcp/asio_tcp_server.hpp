#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class AsioTCPServer {
public:
    AsioTCPServer(asio::io_context& io_context, unsigned short port);
    ~AsioTCPServer();

    void start(int numThreads = 0);
    void stop();

private:
    void acceptConnections();
    void handleAccept(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error);
    void readFromClient(std::shared_ptr<tcp::socket> socket);
    void handleRead(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<std::vector<char>> buffer);
    void handleWrite(std::shared_ptr<tcp::socket> socket, const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<std::vector<char>> buffer);

    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::thread> workerThreads_;
    std::atomic<bool> running_ = false;
    unsigned short port_;
};