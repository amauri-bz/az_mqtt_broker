#pragma once

#include <span>
#include <cstdint>
#include <optional>
#include "az_globals.hpp"
#include "az_db_intf.hpp"
#include "az_outbound_pool.hpp"
#include "az_connect_intf.hpp"

namespace AzMqttBroker {

/**
 * @brief
 */
class MqttProtocolHandler {
public:
    ~MqttProtocolHandler() = default;
    static void handle(MqttPacketContext& ctx,
                                std::shared_ptr<DbIntf> dbMgr,
                                std::shared_ptr<OutboundPoolIntf> out);

    static void process_connect(MqttPacketContext& ctx,
                                            std::span<const char> payload,
                                            std::shared_ptr<DbIntf> dbMgr,
                                            std::shared_ptr<OutboundPoolIntf> out);

     static std::string read_mqtt_string(std::span<const char>& data);

    static std::optional<FixedHeader> decode_fixed_header(std::span<const char> data);
};
}