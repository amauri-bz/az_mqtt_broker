#pragma once

#include <string>
#include <memory>

#include "az_globals.hpp"

namespace AzMqttBroker {

/**
 * @brief
 */
class DbIntf {
public:
    virtual ~DbIntf() = default;
    virtual void save_session(const std::string& clientId, int fd) = 0;
    virtual std::shared_ptr<ClientSession> get_session(const std::string& clientId) = 0;
    virtual std::shared_ptr<ClientSession> get_session_by_fd(int fd) = 0;
    virtual void remove_session(int fd) = 0;
};
}