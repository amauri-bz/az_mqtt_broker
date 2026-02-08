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
    virtual std::vector<std::string> split_topic(const std::string& topic) = 0;
    virtual void subscribe(const std::string& clientId, const std::string& topicFilter) = 0;
    virtual std::vector<std::string> get_subscribers_for_topic(const std::string& topic) = 0;
};
}