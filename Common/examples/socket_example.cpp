#include "time_utils.hpp"
#include "logging.hpp"
#include "tcp_server.hpp"

int main(int,char**){
    using namespace Common;
    std::string time_str_;
    Logger logger_("socket_example.log");

    auto tcpRecvCallback = [&logger_](TCPSocket* socket, Nanos rx_time) noexcept {
        logger_.log("TCPServer::defaultRecvCallback() socket:% len:% rx:%\n", socket->getFD(), socket->next_valid_read_idx_, rx_time);

        const std::string reply = "TCPServer received msg: " + std::string(socket->inbound_data_.data(), socket->next_valid_read_idx_);
        socket->next_valid_read_idx_ = 0;

        socket->send(reply.data(), reply.length());
    };

    auto tcpServerRecvFinishedCallback = [&logger_]() noexcept {
        logger_.log("TCPServer::defaultRecvFinishedCallback()\n");
    };

    auto tcpClientRecvCallback = [&logger_](TCPSocket *socket, Nanos rx_time) noexcept {
        const std::string recv_msg = std::string(socket->inbound_data_.data(), socket->next_valid_write_idx_);
        socket->next_valid_write_idx_ = 0;
        logger_.log("TCPSocket::defaultRecvCallback() socket:% len:% rx:% msg:%\n", socket->getFD(), socket->next_valid_write_idx_, rx_time, recv_msg);
    };

    const std::string iface = "lo";
    const std::string ip = "127.0.0.1";
    const int port = 12345;

    logger_.log("Creating TCPServer on iface:% port:%\n", iface, port);

    TCPServer server(logger_);
    server.recv_callback_ = tcpRecvCallback;
    server.recv_finished_callback_ = tcpServerRecvFinishedCallback;
    server.listen(iface, port);

    std::vector<TCPSocket*> clients(5);

    for(size_t i = 0; i < clients.size(); i++){
        clients[i] = new TCPSocket(logger_);
        clients[i]->setCallback(tcpClientRecvCallback);
        logger_.log("Connecting TCPClient-[%] on ip:% iface:% port:%\n", i, ip, iface, port);
        clients[i]->connect(ip, iface, port, false);
        server.poll();
    }

    using namespace std::literals::chrono_literals;

    for(int itr = 0; itr < 5; itr++){
        for(int i = 0; i < 5; i++){
            const std::string client_msg = "CLIENT-[" + std::to_string(i) + "] : Sending " + std::to_string(itr * 100 + i);
            logger_.log("Sending TCPClient-[%] %\n", i, client_msg);
            clients[i]->send(client_msg.data(), client_msg.length());
            clients[i]->sendAndRecv();

            std::this_thread::sleep_for(500ms);
            server.poll();
            server.sendAndRecv();
        }
    }

    return 0;
}