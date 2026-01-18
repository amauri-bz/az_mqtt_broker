#pragma once

#include <thread>
#include <memory>
#include "../include/az_connect_mgr.hpp"

namespace AzMqttBroker {

class MqttListener {
private:
    std::thread listener_thread;
    bool running;
    std::shared_ptr<ConnectMgr> connectMgr;
    int server_fd;
public:
    ~MqttListener();
    MqttListener(std::shared_ptr<ConnectMgr> connectMgr, int server_fd);
    void run_loop();
};

}