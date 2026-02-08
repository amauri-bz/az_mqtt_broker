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

    std::cout << "Client connected: " << clientId << std::endl;

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

void MqttProtocolHandler::process_publish(MqttPacketContext& ctx,
                                          std::span<const char> payload,
                                          std::shared_ptr<DbIntf> dbMgr,
                                          std::shared_ptr<OutboundPoolIntf> outboundPool) {
    auto data = payload;

    // Extracting the prot name
    std::string topic = read_mqtt_string(data);

    // Extracting the Packet Identifier (2 bytes)
    uint8_t flags = data[0];
    uint8_t qos = (flags >> 1) & 0x03;
    uint16_t packetId = 0;
    if (qos > 0) {
        if (data.size() < 2) return;
        packetId = (static_cast<uint8_t>(data[0]) << 8) | static_cast<uint8_t>(data[1]);
        data = data.subspan(2);
    }

    auto messagePayload = std::make_shared<std::vector<char>>(data.begin(), data.end());

    std::vector<char> packet;

    // 1. Fixed Header para PUBLISH (QoS 0)
    packet.push_back(0x30);

    size_t remainingLength = 2 + topic.size() + (*messagePayload).size();
    packet.push_back(static_cast<char>(remainingLength));

    // 3. Topic Name
    packet.push_back(0x00);
    packet.push_back(static_cast<char>(topic.size()));
    packet.insert(packet.end(), topic.begin(), topic.end());

    // 4. Payload
    packet.insert(packet.end(), (*messagePayload).begin(), (*messagePayload).end());

    auto subscribers = dbMgr->get_subscribers_for_topic(topic);

    for (const auto& subId : subscribers) {
        auto subSession = dbMgr->get_session(subId);
        if (subSession) {
            std::cout << "process_publish Session: " << subSession->clientId << " Topic:" << topic << "\n";
            MqttPacketContext outCtx;
            outCtx.raw_data = std::make_shared<std::vector<char>>(packet.begin(), packet.end());
            outCtx.socket_fd = subSession->socket_fd;

            subSession->queue.push(std::move(outCtx));
            outboundPool->notify_client_ready(subId);
        }
    }
}

void MqttProtocolHandler::send_suback(const std::string& clientId, uint16_t packetId,
                 const std::vector<uint8_t>& qosLevels,
                 std::shared_ptr<DbIntf> dbMgr, std::shared_ptr<OutboundPoolIntf> outboundPool) {

    auto session = dbMgr->get_session(clientId);
    if (!session) return;

    // Header: 0x90 (Type 9), size (2 + number of QoS's)
    std::vector<char> subackData;
    subackData.push_back(static_cast<char>(0x90));
    subackData.push_back(static_cast<char>(2 + qosLevels.size()));

    // Packet ID (MSB, LSB)
    subackData.push_back(static_cast<char>(packetId >> 8));
    subackData.push_back(static_cast<char>(packetId & 0xFF));

    // Payload: List of QoS's
    for (uint8_t qos : qosLevels) {
        subackData.push_back(static_cast<char>(qos));
    }

    auto response = std::make_shared<std::vector<char>>(std::move(subackData));

    MqttPacketContext ackCtx;
    ackCtx.raw_data = response;
    ackCtx.socket_fd = session->socket_fd;

    session->queue.push(std::move(ackCtx));
    outboundPool->notify_client_ready(clientId);
}

void MqttProtocolHandler::process_subscribe(MqttPacketContext& ctx,
                                           std::span<const char> payload,
                                           std::shared_ptr<DbIntf> dbMgr,
                                           std::shared_ptr<OutboundPoolIntf> outboundPool) {
    auto data = payload;

    if (data.size() < 2) return;
    uint16_t packetId = (static_cast<uint8_t>(data[0]) << 8) | static_cast<uint8_t>(data[1]);
    data = data.subspan(2);

    std::vector<uint8_t> grantedQoS;

    auto session = dbMgr->get_session_by_fd(ctx.socket_fd);

    if(!session) return;

    while (!data.empty()) {
        std::string topicFilter = read_mqtt_string(data);
        if (topicFilter.empty()) break;

        if (data.empty()) break;
        uint8_t requestedQoS = static_cast<uint8_t>(data[0]);
        data = data.subspan(1);

        dbMgr->subscribe(session->clientId, topicFilter);

        // get QoS (0, 1 ou 2)
        grantedQoS.push_back(requestedQoS);

        std::cout << "Cliente [" << session->clientId << "] subscribed to topic: " << topicFilter << std::endl;
    }

    // 4. send SUBACK to the client
    send_suback(session->clientId, packetId, grantedQoS, dbMgr, outboundPool);
}

void MqttProtocolHandler::process_disconnect(MqttPacketContext& ctx,
                                            std::shared_ptr<DbIntf>  dbMgr) {
    std::cout << "Cliente FD: [" << ctx.socket_fd << "] disconnected" << std::endl;

    dbMgr->remove_session(ctx.socket_fd);

    close(ctx.socket_fd);
}

void MqttProtocolHandler::handle(MqttPacketContext& ctx,
                                std::shared_ptr<DbIntf> dbMgr,
                                std::shared_ptr<OutboundPoolIntf> outPool) {

    print_hex_buffer(*ctx.raw_data, "ProtocolHandler>> ");

    std::span<const char> data_view(*ctx.raw_data);

    auto header = MqttProtocolHandler::decode_fixed_header(data_view);
    if (!header) return;

    std::span<const char> payload = data_view.subspan(header->headerSize);

    switch (header->type) {
        case MqttPacketType::CONNECT:
            std::cout << "MQTT protocol handler: CONNECT\n";
            process_connect(ctx, payload, dbMgr, outPool);
            break;
        case MqttPacketType::PUBLISH:
            std::cout << "MQTT protocol handler: PUBLISH\n";
            process_publish(ctx, payload, dbMgr, outPool);
            break;
        case MqttPacketType::SUBSCRIBE:
            std::cout << "MQTT protocol handler: SUBSCRIBE\n";
            process_subscribe(ctx, payload, dbMgr, outPool);
            break;
         case MqttPacketType::DISCONNECT:
            std::cout << "MQTT protocol handler: DISCONNECT\n";
            process_disconnect(ctx, dbMgr);
            break;
        default:
            std::cout << "Invalid header->type:" << static_cast<int>(header->type) << "\n";
    }
}

}