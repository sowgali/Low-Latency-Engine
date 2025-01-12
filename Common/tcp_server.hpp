#pragma once

#include <sys/epoll.h>

#include "tcp_socket.hpp"


namespace Common {
    class TCPServer {
        private:
            int epoll_fd_ = -1;

            Logger& logger_;
            TCPSocket listener_socket_;

            epoll_event events_[1024];
            std::vector<TCPSocket*> receive_sockets_, send_sockets_;

            std::string time_str_;

            auto addToEpollList(TCPSocket* socket);
        
        public:
            std::function<void(TCPSocket* s, Nanos rx_time)> recv_callback_ = nullptr;
            std::function<void()> recv_finished_callback_ = nullptr;
            
            explicit TCPServer(Logger& logger) : logger_(logger), listener_socket_(logger) {}

            auto listen(const std::string& iface, int port) -> void;

            auto poll() noexcept -> void;

            auto sendAndRecv() noexcept -> void;
    };
}