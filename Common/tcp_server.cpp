#include "tcp_server.hpp"

namespace Common {
    auto TCPServer::addToEpollList(TCPSocket* socket){
        epoll_event ev{EPOLLET | EPOLLIN, {reinterpret_cast<void*>(socket)}};

        return !epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->getFD(), &ev);
    }

    auto TCPServer::listen(const std::string& iface, int port) -> void {
        epoll_fd_ = epoll_create(1);

        ASSERT(epoll_fd_ != 1, "epoll_create() failed error: " + std::string(strerror(errno)));

        ASSERT(listener_socket_.connect("", iface, port, true) >= 0, "listener socket failed to connect. iface: " + iface + " port: " + std::to_string(port) + " error: " + std::string(strerror(errno)));

        ASSERT(addToEpollList(&listener_socket_), "epoll_ctl() failed. errno: " + std::string(strerror(errno)));
    }

    auto TCPServer::sendAndRecv() noexcept -> void {
        std::for_each(send_sockets_.begin(), send_sockets_.end(), [](auto socket){
            socket->sendAndRecv();
        });

        auto recv = false;

        std::for_each(receive_sockets_.begin(), receive_sockets_.end(), [&recv](auto socket){
            recv |= socket->sendAndRecv();
        });

        if(recv){
            recv_finished_callback_();
        }
    }

    auto TCPServer::poll() noexcept -> void {
        const int max_events_ = 1 + send_sockets_.size() + receive_sockets_.size();

        const int n = epoll_wait(epoll_fd_, events_, max_events_, 0);

        bool have_new_connections_ = false;

        for(int i = 0; i < n; i++){
            const auto& event = events_[i];
            auto socket = reinterpret_cast<TCPSocket*>(event.data.ptr);

            if(event.events & EPOLLIN){
                if(socket == &listener_socket_){
                    logger_.log("%:% %() % EPOLLIN listener_socket_: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), socket->getFD());
                    have_new_connections_ = true;
                    continue;
                }
                logger_.log("%:% %() % EPOLLIN socket: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), socket->getFD());

                if(std::find(receive_sockets_.begin(), receive_sockets_.end(), socket) == receive_sockets_.end()){
                    receive_sockets_.push_back(socket);
                }
            }

            if(event.events & EPOLLOUT){
                if(std::find(send_sockets_.begin(), send_sockets_.end(), socket) == send_sockets_.end())
                    send_sockets_.push_back(socket);
            }
        }

        if(have_new_connections_){
            logger_.log("%:% %() % have_new_connection\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));

            sockaddr_storage addr;
            socklen_t addr_len = sizeof(addr);

            int fd = accept(listener_socket_.getFD(), reinterpret_cast<sockaddr*>(&addr), &addr_len);
            
            if(fd >= 0) [[likely]] {
                ASSERT(setNonBlocking(fd) && disableNagle(fd), "Failed to set non-blocking or disabling Nagle on socket: " + std::to_string(fd));

                logger_.log("%:% %() % accept new connection: %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), fd);

                auto new_socket = new TCPSocket(logger_);
                new_socket->setFD(fd);
                new_socket->setCallback(recv_callback_);

                ASSERT(addToEpollList(new_socket), "Unable to add socket. error: " + std::string(strerror(errno)));

                if(std::find(receive_sockets_.begin(), receive_sockets_.end(), new_socket) == receive_sockets_.end()){
                    receive_sockets_.push_back(new_socket);
                }
            }
        }
    }
}