#pragma once

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <set>
#include "az_outbound_pool_intf.hpp"
#include "az_db_mgr.hpp"
#include "az_connect_mgr.hpp"

namespace AzMqttBroker {

class OutboundPool : public OutboundPoolIntf {
private:
    std::vector<std::jthread> workers;

    std::queue<std::string> delivery_queue;
    std::set<std::string> active_clients;

    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;

    std::shared_ptr<DbIntf> dbMgr;
    std::shared_ptr<ConnectIntf> connectMgr;

public:
    OutboundPool(size_t threads, std::shared_ptr<DbIntf> db, std::shared_ptr<ConnectIntf> conn);

    void notify_client_ready(const std::string& clientId) override;

private:
    void delivery_loop(std::stop_token st);

    void process_client_delivery(const std::string& clientId);
};
}
