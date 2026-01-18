#pragma once

#include <vector>

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
};
}