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

void MqttProtocolHandler::process_packet(MqttPacketContext ctx) {

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
            // process_connect(ctx, payload);
            break;
        case MqttPacketType::PUBLISH:
            // process_publish(ctx, payload, header->flags);
            break;
        // ... outros casos
    }
}

void MqttProtocolHandler::handle(MqttPacketContext& ctx, std::shared_ptr<DbIntf> dbMgr, std::shared_ptr<OutboundPoolIntf> out) {

    std::cout << "MQTT protocol handler\n";

    process_packet(ctx);

    dbMgr->save_session("subscriber1", ctx.socket_fd);

    auto session = dbMgr->get_session("subscriber1");
    if (session) {
        session->queue.push(ctx);
        out->notify_client_ready("subscriber1");
    }
}

}