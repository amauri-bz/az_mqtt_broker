#pragma once

#include <span>
#include <cstdint>
#include <optional>
#include "az_globals.hpp"
#include "az_db_intf.hpp"
#include "az_outbound_pool.hpp"

namespace AzMqttBroker {

/**
 * @brief
 */
class MqttProtocolHandler {
public:
    ~MqttProtocolHandler() = default;
    static void handle(MqttPacketContext& ctx, std::shared_ptr<DbIntf> dbMgr, std::shared_ptr<OutboundPoolIntf> out);
    static void process_packet(MqttPacketContext ctx);
    static std::optional<FixedHeader> decode_fixed_header(std::span<const char> data);
};
}