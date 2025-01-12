#pragma once

#include<functional>

#include "socket_utils.hpp"
#include "macros.hpp"

namespace Common {
    constexpr int TCPBufferSize = 64 * 1024 * 1024;

    class TCPSocket {
        typedef std::function<void(TCPSocket* s, Nanos rx_time)> CallbackType;
        private:
            int socket_fd_ = -1;

            struct sockaddr_in sock_attrib_{};

            CallbackType recv_callback_ = nullptr;

            std::string time_str_;

            Logger& logger_;
        
        public:
            std::vector<char> outbound_data_;
            size_t next_valid_write_idx_ = 0;
            std::vector<char> inbound_data_;
            size_t next_valid_read_idx_ = 0;
            
            explicit TCPSocket(Logger& logger) : logger_(logger) {
                outbound_data_.resize(TCPBufferSize);
                inbound_data_.resize(TCPBufferSize);
            }

            inline auto getFD() noexcept -> int {
                return socket_fd_;
            }

            inline auto setFD(int fd) noexcept -> void {
                socket_fd_ = fd;
            }

            inline auto setCallback(CallbackType callback) noexcept -> void {
                recv_callback_ = callback;
            }

            auto connect(const std::string& ip, const std::string& iface, int port, bool is_listening_) -> int;

            auto sendAndRecv() noexcept -> bool;

            auto send(const void* data, size_t len) noexcept -> void;

            TCPSocket() = delete;
            TCPSocket(const TCPSocket&) = delete;
            TCPSocket(const TCPSocket&&) = delete;
            TCPSocket& operator=(const TCPSocket&) = delete;
            TCPSocket& operator=(const TCPSocket&&) = delete;
    };
}