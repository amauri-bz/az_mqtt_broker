#include <iostream>
#include "../include/az_mqtt_protocol_handler.hpp"

namespace AzMqttBroker {

std::optional<FixedHeader> MqttProtocolHandler::decode_fixed_header(std::span<const char> data) {
    if (data.size() < 2) return std::nullopt;

    FixedHeader header;
    // Byte 1: [type (4 bits)][Flags (4 bits)]
    uint8_t firstByte = static_cast<uint8_t>(data[0]);
    header.type = static_cast<MqttPacketType>(firstByte >> 4);
    header.flags = firstByte & 0x0F;

    size_t multiplier = 1;
    size_t value = 0;
    size_t index = 1;
    uint8_t encodedByte;

    do {
        if (index >= data.size()) return std::nullopt;
        encodedByte = static_cast<uint8_t>(data[index++]);
        value += (encodedByte & 127) * multiplier;
        if (multiplier > 128 * 128 * 128) return std::nullopt;
        multiplier *= 128;
    } while ((encodedByte & 128) != 0);

    header.remainingLength = value;
    header.headerSize = index;
    return header;
}

std::string MqttProtocolHandler::read_mqtt_string(std::span<const char>& data) {
    if (data.size() < 2) return "";

    // First 2 bytes is the lengh (Big Endian)
    uint16_t len = (static_cast<uint8_t>(data[0]) << 8) | static_cast<uint8_t>(data[1]);

    if (data.size() < 2 + len) return "";

    std::string str(data.data() + 2, len);
    data = data.subspan(2 + len); // Avança o span
    return str;
}

void MqttProtocolHandler::process_connect(MqttPacketContext& ctx,
                                         std::span<const char> payload,
                                         std::shared_ptr<DbIntf> dbMgr,
                                         std::shared_ptr<OutboundPoolIntf> outPool) {
    auto data = payload;

    // Protocol Name (MQTT)
    std::string protocolName = read_mqtt_string(data);
    if (protocolName != "MQTT") {
        // Error: Invalid Protocol
        return;
    }

    // Protocol Level (v3.1.1 é 4)
    if (data.empty()) return;
    uint8_t level = data[0];
    data = data.subspan(1);

    // Connect Flags
    if (data.empty()) return;
    uint8_t flags = data[0];
    bool cleanSession = (flags >> 1) & 0x01;
    data = data.subspan(1);

    // Keep Alive (2 bytes)
    if (data.size() < 2) return;
    uint16_t keepAlive = (static_cast<uint8_t>(data[0]) << 8) | static_cast<uint8_t>(data[1]);
    data = data.subspan(2);

    // Payload: Client ID
    std::string clientId = read_mqtt_string(data);
    if (clientId.empty() && !cleanSession) {
        // Erro: ClientId empty is only allowed in case of CleanSession = 1
        return;
    }

    // Save data in the session DB
    dbMgr->save_session(clientId, ctx.socket_fd);

    std::cout << "Client connected: " << clientId << " (CleanSession: " << cleanSession << ")" << std::endl;

    // CONNACK msg creation
    auto connack_data = std::make_shared<std::vector<char>>(
        std::vector<char>{ 0x20, 0x02, 0x00, 0x00 }
    );

    // Put the ACK in the same client session outbound
    auto session = dbMgr->get_session(clientId);
    if (session) {
        MqttPacketContext response;
        response.raw_data = connack_data;
        response.socket_fd = ctx.socket_fd;

        session->queue.push(response);

        // Notify the OutboundPool
        outPool->notify_client_ready(clientId);
    }
}

void MqttProtocolHandler::handle(MqttPacketContext& ctx,
                                std::shared_ptr<DbIntf> dbMgr,
                                std::shared_ptr<OutboundPoolIntf> outPool) {

    std::cout << "MQTT protocol handler\n";

     std::span<const char> data_view(*ctx.raw_data);

    auto header = MqttProtocolHandler::decode_fixed_header(data_view);
    if (!header) return;

    std::span<const char> payload = data_view.subspan(header->headerSize);

    std::cout << "MQTT packet=>"
            << " type:" << static_cast<int>(header->type)
            << " flags:" << static_cast<int>(header->flags)
            << " remainingLength:" << header->remainingLength
            << " headerSize:" << header->headerSize << "\n";

    switch (header->type) {
        case MqttPacketType::CONNECT:
            process_connect(ctx, payload, dbMgr, outPool);
            break;
        case MqttPacketType::PUBLISH:
            // process_publish(ctx, payload, header->flags);
            break;
    }
}

}