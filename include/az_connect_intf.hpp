#pragma once

#include <vector>
#include <memory>

namespace AzMqttBroker {

/**
 * @brief
 */
class ConnectIntf {
public:
    virtual ~ConnectIntf() = default;
    virtual void set_nonblocking(int fd) = 0;
    virtual void add_socket(int fd) = 0;
    virtual std::vector<int> wait_for_events() = 0;
    virtual int setup_server_socket(int port) = 0;
    virtual int accept_socket(int fd) = 0;
    virtual ssize_t socket_recv(int fd, std::shared_ptr<std::vector<char>> buffer) = 0;
    virtual ssize_t socket_send(int fd, std::shared_ptr<std::vector<char>> buffer) = 0;
};
}