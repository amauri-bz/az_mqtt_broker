#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <set>

#include "../include/az_outbound_pool.hpp"

namespace AzMqttBroker {

OutboundPool::OutboundPool(size_t threads, std::shared_ptr<DbIntf> db, std::shared_ptr<ConnectIntf> conn) : dbMgr(db), connectMgr(conn) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this](std::stop_token st) {
            this->delivery_loop(st);
        });
    }
}

void OutboundPool::notify_client_ready(const std::string& clientId) {
    {
        std::lock_guard lock(mtx);
        if (active_clients.find(clientId) == active_clients.end()) {
            delivery_queue.push(clientId);
            active_clients.insert(clientId);
        }
    }
    cv.notify_one();
}


void OutboundPool::delivery_loop(std::stop_token st) {
    while (!st.stop_requested()) {
        std::string clientId;
        {
            std::unique_lock lock(mtx);
            cv.wait(lock, [this, &st] {
                return stop || !delivery_queue.empty() || st.stop_requested();
            });

            if (stop || st.stop_requested()) return;

            clientId = std::move(delivery_queue.front());
            delivery_queue.pop();
        }

        process_client_delivery(clientId);

        {
            std::lock_guard lock(mtx);
            active_clients.erase(clientId);
        }
    }
}

void OutboundPool::process_client_delivery(const std::string& clientId) {
    auto session = dbMgr->get_session(clientId);
    if (!session) return;

    while (true) {
        auto msg_opt = session->queue.try_pop();
        if (!msg_opt) break;

        connectMgr->socket_send(session->socket_fd, msg_opt->raw_data);
    }
}

} //namespace