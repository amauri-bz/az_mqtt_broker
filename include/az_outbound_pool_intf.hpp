#pragma once

#include <condition_variable>
#include <functional>

namespace AzMqttBroker {

/**
 * @brief
 */
class OutboundPoolIntf {
public:
   virtual ~OutboundPoolIntf() = default;
   virtual void notify_client_ready(const std::string& clientId) = 0;
};
}
