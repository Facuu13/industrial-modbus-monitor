import os
import json
import time
from pathlib import Path

import paho.mqtt.client as mqtt

MQTT_HOST = os.getenv("MQTT_HOST", "192.168.1.11")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "industrial/modbus/+/telemetry")

OUT_DIR = Path(os.getenv("OUT_DIR", "/data"))
OUT_DIR.mkdir(parents=True, exist_ok=True)
OUT_FILE = OUT_DIR / "telemetry.jsonl"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"[MQTT] connected {MQTT_HOST}:{MQTT_PORT}")
        client.subscribe(MQTT_TOPIC)
        print(f"[MQTT] subscribed {MQTT_TOPIC}")
    else:
        print(f"[MQTT] connect failed rc={rc}")

def on_message(client, userdata, msg):
    payload_raw = msg.payload.decode("utf-8", errors="replace")
    try:
        data = json.loads(payload_raw)
    except json.JSONDecodeError:
        print(f"[BAD JSON] topic={msg.topic} payload={payload_raw}")
        return

    rec = {
        "ts_ingest": int(time.time()),
        "topic": msg.topic,
        "data": data,
    }

    with OUT_FILE.open("a", encoding="utf-8") as f:
        f.write(json.dumps(rec, ensure_ascii=False) + "\n")

    print(f"[SAVED] {msg.topic} device={data.get('device_id')} level={data.get('level')}")

def main():
    client = mqtt.Client(client_id="industrial-listener")
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_HOST, MQTT_PORT, keepalive=30)
    client.loop_forever()

if __name__ == "__main__":
    main()
