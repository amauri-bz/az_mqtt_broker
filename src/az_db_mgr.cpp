#include <iostream>
#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <optional>
#include <mutex>
#include "../include/az_db_mgr.hpp"

namespace AzMqttBroker {

void DbMgr::save_session(const std::string& clientId, int fd) {
    std::unique_lock lock(mtx);

    auto session = std::make_shared<ClientSession>();
    session->clientId = clientId;
    session->socket_fd = fd;

    sessions[clientId] = session;
    fd_to_clientId[fd] = clientId;
}

std::shared_ptr<ClientSession> DbMgr::get_session(const std::string& clientId) {
    std::shared_lock lock(mtx);
    auto it = sessions.find(clientId);
    return (it != sessions.end()) ? it->second : nullptr;
}

std::shared_ptr<ClientSession> DbMgr::get_session_by_fd(int fd) {
    std::shared_lock lock(mtx);
    auto it_id = fd_to_clientId.find(fd);
    if (it_id == fd_to_clientId.end()) return nullptr;

    auto it_sess = sessions.find(it_id->second);
    return (it_sess != sessions.end()) ? it_sess->second : nullptr;
}

void DbMgr::remove_session(int fd) {
    std::unique_lock lock(mtx);
    auto it_id = fd_to_clientId.find(fd);
    if (it_id != fd_to_clientId.end()) {
        sessions.erase(it_id->second);
        fd_to_clientId.erase(it_id);
    }
}

}