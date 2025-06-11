import json
import ssl
import time
import random
import paho.mqtt.client as mqtt

# Definindo o endereço do broker
broker = 'broker.emqx.io'
port = 1883
topic = "python/mqtt"
client_id = f'python-mqtt-{random.randint(0, 1000)}'
# username = 'emqx'
# password = 'public'
#--------------------------------

# ---- Configurações da Arduino Cloud ----
DEVICE_ID = "1badc53e-aac9-46c3-bf4d-de6a8cda25f0"  # ID do dispositivo na Arduino Cloud
SECRET_KEY = "Q8WQPLacRvoi0uxBsRYeAw5UV"
TOPIC = f"/v1/devices/{DEVICE_ID}/events"

# ---- Criação do cliente MQTT ----
client = mqtt.Client(client_id=DEVICE_ID)
client.username_pw_set(DEVICE_ID, SECRET_KEY)

# ---- Conexão segura via TLS ----
client.tls_set(tls_version=ssl.PROTOCOL_TLSv1_2)
client.connect("mqtts-sa.iot.arduino.cc", 8883)

# ---- Enviando dados simulados ----
def envia_dados():
    while True:
        temperatura = round(random.uniform(20, 35), 2)
        
        payload = {
            "temperaturaSensor": temperatura  # nome EXATO da variável criada na Arduino Cloud
        }

        print(f"Enviando: {payload}")
        client.publish(TOPIC, json.dumps(payload))
        time.sleep(5)

# ---- Loop principal ----
client.loop_start()
envia_dados()
