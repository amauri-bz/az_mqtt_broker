import paho.mqtt.client as mqtt
import sys

def on_connect(client, userdata, flags, rc):
    print("Connected with code:", rc)
    client.subscribe(topic)

def on_message(client, userdata, msg):
    print(f"Mesage received: {msg.topic} -> {msg.payload.decode()}")

if len(sys.argv) < 3:
    print("Use: python3 subscriber.py <clientID> <topico>")
    sys.exit(1)

client_id = sys.argv[1]
topic = sys.argv[2]

client = mqtt.Client(client_id)
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()
