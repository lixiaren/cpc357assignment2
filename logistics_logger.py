import paho.mqtt.client as mqtt
import pymongo
import json
import datetime

# config
MQTT_TOPIC = "coldchain/telemetry"
MONGO_DB_NAME = "logistics_db"
MONGO_COLLECTION = "shipment_logs"

# Connect MongoDB
mongo_client = pymongo.MongoClient("mongodb://localhost:27017/")
db = mongo_client[MONGO_DB_NAME]
collection = db[MONGO_COLLECTION]

# after connect to MQTT Broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT Broker (Code: {rc})")
    client.subscribe(MQTT_TOPIC)

# when receive message
def on_message(client, userdata, msg):
    try:
        payload_str = msg.payload.decode('utf-8')
        print(f"Received: {payload_str}")

        # JSON
        data = json.loads(payload_str)

        # add server timestamp
        data["server_timestamp"] = datetime.datetime.now()

        # save in MongoDB
        collection.insert_one(data)
        print(" -> Saved to MongoDB successfully!")

        if data.get("status") == "BREACH":
            print(" [ALERT] Temperature Violation Detected! Email sent to admin.")

    except Exception as e:
        print(f"Error: {e}")

# start client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("admin", "1234")
print("Starting Cold Chain Logger Service...")
client.connect("localhost", 1883, 60)
client.loop_forever()