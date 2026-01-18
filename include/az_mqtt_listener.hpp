#pragma once

#include <thread>
#include <memory>
#include "../include/az_connect_mgr.hpp"
#include "../include//az_worker_pool.hpp"

namespace AzMqttBroker {

class MqttListener {
private:
    std::thread listener_thread;
    bool running;
    std::shared_ptr<ConnectIntf> connectMgr;
    std::shared_ptr<ThreadPoolIntf> workerPool;
    int server_fd;
public:
    ~MqttListener();
    MqttListener(std::shared_ptr<ConnectIntf> connectMgr, std::shared_ptr<ThreadPoolIntf> workerPool, int server_fd);
    void run_loop();
};

}