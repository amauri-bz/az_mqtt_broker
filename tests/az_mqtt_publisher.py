import paho.mqtt.client as mqtt
import sys
import time

def on_connect(client, userdata, flags, rc):
    print("Conected with code:", rc)
    client.publish(topic, message)

if len(sys.argv) < 4:
    print("Use: python3 publisher.py <clientID> <topico> <mensagem>")
    sys.exit(1)

client_id = sys.argv[1]
topic = sys.argv[2]
message = sys.argv[3]

client = mqtt.Client(client_id)
client.on_connect = on_connect

client.connect("localhost", 1883, 60)
client.loop_start()

time.sleep(2)
client.disconnect()
