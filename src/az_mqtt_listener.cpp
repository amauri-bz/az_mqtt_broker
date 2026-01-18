
#include <iostream>
#include "../include/az_mqtt_listener.hpp"
#include "../include/az_connect_mgr.hpp"

namespace AzMqttBroker {

MqttListener::~MqttListener() {
    running = false;
}

MqttListener::MqttListener(std::shared_ptr<ConnectMgr> conn, int fd) {
    running = true;
    connectMgr = conn;
    server_fd = fd;
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
            }
        }
    }
}

}