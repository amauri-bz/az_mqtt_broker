#pragma once

#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <optional>
#include <condition_variable>

namespace AzMqttBroker
{

struct MqttPacketContext {
    std::shared_ptr<std::vector<char>> raw_data;
    int socket_fd;
};

enum class MqttPacketType : uint8_t {
    CONNECT = 1,
    CONNACK = 2,
    PUBLISH = 3,
    PUBACK = 4,
    SUBSCRIBE = 8,
    SUBACK = 9,
    PINGREQ = 12,
    PINGRESP = 13,
    DISCONNECT = 14
};

struct FixedHeader {
    MqttPacketType type;
    uint8_t flags;
    size_t remainingLength;
    size_t headerSize;
};

class ClientQueue {
    std::queue<MqttPacketContext> messages;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(MqttPacketContext msg) {
        {
            std::lock_guard lock(mtx);
            messages.push(std::move(msg));
        }
        cv.notify_one();
    }

    MqttPacketContext pop_wait() {
        std::unique_lock lock(mtx);
        cv.wait(lock, [this] { return !messages.empty(); });
        auto msg = std::move(messages.front());
        messages.pop();
        return msg;
    }

    std::optional<MqttPacketContext> try_pop() {
        std::lock_guard lock(mtx);
        if (messages.empty()) return std::nullopt;
        auto msg = std::move(messages.front());
        messages.pop();
        return msg;
    }
};

struct ClientSession {
    std::string clientId;
    int socket_fd;
    ClientQueue queue;
};

}