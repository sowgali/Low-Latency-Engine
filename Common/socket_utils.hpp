#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <netdb.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "macros.hpp"
#include "logging.hpp"


namespace Common {

    struct socketConfig {
        std::string ip_;
        std::string iface_;
        int port_ = -1;
        bool is_udp_ = false;
        bool is_listening_ = false;
        bool needs_so_timestamp_ = false;

        auto toString() const {
            std::stringstream ss;
            ss << "SocketCfg[ip:" << ip_
            << " iface:" << iface_
            << " port:" << port_
            << " is_udp:" << is_udp_
            << " is_listening:" << is_listening_
            << " needs_SO_timestamp:" << needs_so_timestamp_
            << "]";
            
            return ss.str();
        }
    };

    constexpr int MaxTCPServerBacklog = 1024;

    /// Convert interface name "eth0" to ip "123.123.123.123".
    auto inline getIfaceIP(const std::string& iface) -> std::string {
        char buf[NI_MAXHOST] = {'\0'};
        ifaddrs* ifaddr = nullptr;

        if(getifaddrs(&ifaddr) != -1) {
            for(ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next){
                if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name){
                    getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
                    break;
                }
            }
            freeifaddrs(ifaddr);
        }
        return buf;
    }

    inline auto setNonBlocking(int fd) -> bool {
        const auto flags = fcntl(fd, F_GETFL, 0);

        if(flags & O_NONBLOCK)
            return true;
        
        return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
    }

    inline auto disableNagle(int fd) -> bool {
        int yes = 1;

        return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const void*>(&yes), sizeof(yes)) != -1);
    }

    inline auto enableSOTimestamp(int fd) -> bool {
        int yes = 1;
        return (setsockopt(fd, IPPROTO_TCP, SO_TIMESTAMP, reinterpret_cast<const void*>(&yes), sizeof(yes)) != -1);
    }

    /// Add / Join membership / subscription to the multicast stream specified and on the interface specified.
    inline auto join(int fd, const std::string &ip) -> bool {
        const ip_mreq mreq{{inet_addr(ip.c_str())}, {htonl(INADDR_ANY)}};
        return (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != -1);
    }

    [[nodiscard]] inline auto createSocket(Logger& logger, const socketConfig& sock_cfg_) -> int {
        std::string time_str;

        const auto ip = sock_cfg_.ip_.empty() ? getIfaceIP(sock_cfg_.iface_) : sock_cfg_.ip_;
        logger.log("%:% %() % cfg:%\n", __FILE__, __LINE__, __FUNCTION__,
               Common::getCurrentTimeStr(&time_str), sock_cfg_.toString());
        
        const int input_flags = (sock_cfg_.is_listening_ ? AI_PASSIVE : 0) | (AI_NUMERICHOST | AI_NUMERICSERV);

        const addrinfo hints{input_flags, AF_INET, sock_cfg_.is_udp_ ? SOCK_DGRAM : SOCK_STREAM, sock_cfg_.is_udp_ ? IPPROTO_UDP : IPPROTO_TCP, 0,0,nullptr,nullptr};
        addrinfo *result = nullptr;
        const auto rc = getaddrinfo(ip.c_str(), std::to_string(sock_cfg_.port_).c_str(), &hints, &result);

        ASSERT(!rc, "getaddrinfo() failed. error:" + std::string(gai_strerror(rc)) + "errno:" + strerror(errno));

        int sock_fd = -1;
        int yes = 1;

        for(addrinfo* rp = result; rp; rp = rp->ai_next){
            ASSERT((sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) != -1, "socket() failed. errno: " + std::string(strerror(errno)));

            ASSERT(setNonBlocking(sock_fd), "setNonBlocking() failed. errno: " + std::string(strerror(errno)));

            if(!sock_cfg_.is_udp_){
                ASSERT(disableNagle(sock_fd), "disableNagle() failed errno: " + std::string(strerror(errno)));
            }

            if(sock_cfg_.is_listening_){
                ASSERT(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void*>(&yes), sizeof(yes)) == 0, "setsockopt() SO_REUSEADDR failed. errno: " + std::string(strerror(errno)));

                const sockaddr_in addr{AF_INET, htons(sock_cfg_.port_), {htonl(INADDR_ANY)}, {}};
                ASSERT(bind(sock_fd, sock_cfg_.is_udp_ ? reinterpret_cast<const struct sockaddr*>(&addr) : rp->ai_addr, sizeof(addr)) == 0, "bind() failed. errno: " + std::string(strerror(errno)));
            } else {
                ASSERT(connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != 0, "connect() failed. errno: " + std::string(strerror(errno)));
            }

            if(!sock_cfg_.is_udp_ && sock_cfg_.is_listening_){
                ASSERT(listen(sock_fd, MaxTCPServerBacklog) == 0, "listen failed(). errno: " + std::string(strerror(errno)));
            }

            if(sock_cfg_.needs_so_timestamp_){
                ASSERT(enableSOTimestamp(sock_fd), "setSOTimestamp failed. errno: " + std::string(strerror(errno)));
            }

        }

        return sock_fd;

    }






}