#pragma once

#include <condition_variable>
#include <functional>

namespace AzMqttBroker {

/**
 * @brief
 */
class ThreadPoolIntf {
public:
    virtual ~ThreadPoolIntf() = default;
    virtual void enqueue(std::function<void()> task) = 0;
    virtual void worker_loop(std::stop_token st) = 0;
};
}
