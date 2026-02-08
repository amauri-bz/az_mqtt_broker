#pragma once

#include <iostream>
#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <functional>

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

    TopicNode root;

public:
    void save_session(const std::string& clientId, int fd) override;

    std::shared_ptr<ClientSession> get_session(const std::string& clientId) override;

    std::shared_ptr<ClientSession> get_session_by_fd(int fd) override;

    void remove_session(int fd) override;

    void subscribe(const std::string& clientId, const std::string& topicFilter) override;

    std::vector<std::string> split_topic(const std::string& topic)  override;

    std::vector<std::string> get_subscribers_for_topic(const std::string& topic) override;

    void unsubscribe_from_all_tree(TopicNode* node, const std::string& clientId);
};
}