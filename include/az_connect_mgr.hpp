#pragma once
#include <vector>
#include "../include/az_connect_intf.hpp"

namespace AzMqttBroker {

/**
 * @brief
 */

class ConnectMgr : public ConnectIntf {
private:
    int epoll_fd;
    static const int MAX_EVENTS = 1024;

public:
    ConnectMgr();
    void set_nonblocking(int fd) override;
    void add_socket(int fd) override;
    std::vector<int> wait_for_events() override;
    int setup_server_socket(int port);
    int accept_socket(int fd);
};
}


