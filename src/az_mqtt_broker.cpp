#include <iostream>
#include <memory>
#include "../include/az_mqtt_broker.hpp"
#include "../include/az_db_mgr.hpp"
#include "../include/az_worker_pool.hpp"
#include "../include/az_outbound_pool.hpp"

namespace AzMqttBroker {

void MqttBroker::start() {
    try {
        auto connectMgr = std::make_shared<ConnectMgr>();
        auto dbMgr = std::make_shared<DbMgr>();
        auto workerPool = std::make_shared<WorkerPool>(8); // 8 threads
        auto OutPool = std::make_shared<OutboundPool>(8, dbMgr, connectMgr); // 8 threads

        int server_fd = connectMgr->setup_server_socket(1883);

        std::cout << "New socket:" << server_fd << "\n";

        connectMgr->add_socket(server_fd);

        MqttListener listener(connectMgr, workerPool, dbMgr, server_fd, OutPool);
        listener.run_loop();

    } catch (const std::exception& e) {
        std::cerr << "FATAL Initialization Error: " << e.what() << std::endl;
        return;
    }
}

} //namespace

int main() {
    AzMqttBroker::MqttBroker mqttBroker{};
    mqttBroker.start();
    return 0;
}


