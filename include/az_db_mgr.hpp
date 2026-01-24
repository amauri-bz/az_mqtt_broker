#pragma once

#include <iostream>
#include <unordered_map>
#include <shared_mutex>
#include <memory>

#include "az_db_intf.hpp"
#include "az_globals.hpp"

namespace AzMqttBroker {

/**
 * @brief
 */
class DbMgr : public DbIntf {
private:
    std::unordered_map<std::string, std::shared_ptr<ClientSession>> sessions;

    std::unordered_map<int, std::string> fd_to_clientId;

    mutable std::shared_mutex mtx;

public:
    void save_session(const std::string& clientId, int fd) override;

    std::shared_ptr<ClientSession> get_session(const std::string& clientId) override;

    std::shared_ptr<ClientSession> get_session_by_fd(int fd) override;

    void remove_session(int fd) override;
};
}