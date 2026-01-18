
#include <iostream>
#include "../include/az_mqtt_listener.hpp"
#include "../include/az_connect_mgr.hpp"

namespace AzMqttBroker {

MqttListener::~MqttListener() {
    running = false;
}

MqttListener::MqttListener(std::shared_ptr<ConnectIntf> conn, std::shared_ptr<ThreadPoolIntf> poll, int fd) {
    running = true;
    connectMgr = conn;
    server_fd = fd;
    workerPool = poll;
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
                    } else if (bytes_read == 0) {
                        //Disconnection handler
                    }
                });
            }
        }
    }
}

}