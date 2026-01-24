
#include <iostream>
#include "../include/az_mqtt_listener.hpp"
#include "../include/az_connect_mgr.hpp"
#include "../include/az_globals.hpp"
#include "../include/az_mqtt_protocol_handler.hpp"
#include "../include/az_db_intf.hpp"

namespace AzMqttBroker {

MqttListener::~MqttListener() {
    running = false;
}

MqttListener::MqttListener(std::shared_ptr<ConnectIntf> conn,
    std::shared_ptr<WorkerPoolIntf> poll,
    std::shared_ptr<DbIntf> db,
    int fd,
    std::shared_ptr<OutboundPoolIntf> out) {
    running = true;
    connectMgr = conn;
    server_fd = fd;
    workerPool = poll;
    dbMgr = db;
    OutPool = out;
}

void MqttListener::run_loop() {
    while (running) {
        std::vector<int> active_fds = connectMgr->wait_for_events();

        for (int fd : active_fds) {
            if (fd == server_fd) {
                std::cout << "New Connection\n";
                int client_fd = connectMgr->accept_socket(fd);
                if (client_fd != -1) {
                    connectMgr->add_socket(client_fd);
                }
            } else {
                std::cout << "Client Message\n";
                workerPool->enqueue([this, fd]() {
                    auto buffer = std::make_shared<std::vector<char>>(2048);
                    ssize_t bytes_read = connectMgr->socket_recv(fd, buffer);

                    if (bytes_read > 0) {
                        std::cout << "Client Message bytes:" << static_cast<int>(bytes_read) << "\n";
                        buffer->resize(bytes_read);
                        MqttPacketContext ctx{ .raw_data = buffer, .socket_fd = fd };
                        MqttProtocolHandler::handle(ctx, dbMgr, OutPool);
                    } else if (bytes_read == 0) {
                        // Disconnection handler
                    }
                });
            }
        }
    }
}

}