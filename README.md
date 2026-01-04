# az_mqtt_broker

## MQTT Broker Implementation in C++20

This project is a lightweight MQTT broker built in C++20, using native sockets and threads. It was designed to support all the main features of an MQTT broker:

- Native TCP socket handling for client connections.
- Thread pools for concurrent message processing and outbound delivery.
- Client session management with subscriptions and per-client queues.
- Pending message store to support QoS 1 and QoS 2 acknowledgments and retransmissions.
- Minimal protocol handler for SUBSCRIBE and PUBLISH operations.
- Extensible design following a modular architecture (ConnectMgr, WorkerPool, OutboundPool, DbMgr, etc.).

## Architecture
The broker is structured around:

- MqttListener: accepts client connections and dispatches incoming packets.
- WorkerPool / WorkerTask: processes inbound messages and applies protocol logic.
- OutboundPool / OutboundTask: manages per-client queues and sends messages to subscribers.
- DbMgr: stores client sessions, subscriptions, and pending messages.
- MqttProtocolHandler: parses and processes MQTT commands.

Currently, it is an ongoing project, in its first preliminary version.
