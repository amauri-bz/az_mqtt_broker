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

    static void process_publish(MqttPacketContext& ctx,
                                          std::span<const char> payload,
                                          std::shared_ptr<DbIntf> dbMgr,
                                          std::shared_ptr<OutboundPoolIntf> outboundPool);

    static std::string read_mqtt_string(std::span<const char>& data);

    static std::optional<FixedHeader> decode_fixed_header(std::span<const char> data);

    static void process_subscribe(MqttPacketContext& ctx,
                                           std::span<const char> payload,
                                           std::shared_ptr<DbIntf> dbMgr,
                                           std::shared_ptr<OutboundPoolIntf> outboundPool);

    static void send_suback(const std::string& clientId, uint16_t packetId,
                 const std::vector<uint8_t>& qosLevels,
                 std::shared_ptr<DbIntf> dbMgr, std::shared_ptr<OutboundPoolIntf> outboundPool);

    static void process_disconnect(MqttPacketContext& ctx,
                                            std::shared_ptr<DbIntf>  dbMgr);
};
}