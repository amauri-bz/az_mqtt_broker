#pragma once

#include <thread>
#include <memory>
#include "../include/az_connect_intf.hpp"
#include "../include/az_db_intf.hpp"
#include "../include/az_worker_pool_intf.hpp"
#include "../include/az_outbound_pool_intf.hpp"

namespace AzMqttBroker {

class MqttListener {
private:
    std::thread listener_thread;
    bool running;
    std::shared_ptr<ConnectIntf> connectMgr;
    std::shared_ptr<WorkerPoolIntf> workerPool;
    std::shared_ptr<DbIntf> dbMgr;
    int server_fd;
    std::shared_ptr<OutboundPoolIntf> OutPool;
public:
    ~MqttListener();
    MqttListener(std::shared_ptr<ConnectIntf> connectMgr,
        std::shared_ptr<WorkerPoolIntf> workerPool,
        std::shared_ptr<DbIntf> dbMgr,
        int server_fd,
        std::shared_ptr<OutboundPoolIntf> OutPool);
    void run_loop();
};

}