# Smart Cold Chain Logistics Tracker ❄️🚛

A secure IoT solution for real-time monitoring of temperature-sensitive logistics using ESP32, MQTT, Google Cloud Platform (GCP), and MongoDB.

## 📋 Table of Contents
1. [GCP Infrastructure Setup](#1-gcp-infrastructure-setup)
2. [Server-Side Configuration (VM)](#2-server-side-configuration-vm)
3. [Middleware Deployment](#3-middleware-deployment)
4. [Hardware Configuration (ESP32)](#4-hardware-configuration-esp32)
5. [Running & Verification](#5-running--verification)

---

## 1. GCP Infrastructure Setup

### Firewall Rules (VPC Network)
Before configuring the server, ensure the following firewall rule is active to allow MQTT traffic:
* **Name:** `allow-mqtt-1883`
* **Direction:** Ingress
* **Source filter:** IPv4 ranges (`0.0.0.0/0`)
* **Protocols and ports:** `tcp:1883`

---

## 2. Server-Side Configuration (VM)

Connect to your GCP VM instance via SSH and execute the following commands.

### 2.1 System Update & Installation
Update the package list and install necessary services (Mosquitto Broker, MongoDB, Python3).

```bash
sudo apt-get update && sudo apt-get upgrade -y
sudo apt-get install -y mosquitto mosquitto-clients
sudo apt-get install -y mongodb
sudo apt-get install -y python3-pip
```
### 2.2 MQTT Security Configuration (Authentication)
We configure Mosquitto to reject anonymous login and require a username/password.

1. **Create Password File** (User: `admin`):
    * *You will be prompted to enter the password (e.g., `1234`) twice.*
    ```bash
    sudo mosquitto_passwd -c /etc/mosquitto/passwd admin
    ```

2. **Configure Access Rules**:
    Open the configuration file:
    ```bash
    sudo nano /etc/mosquitto/mosquitto.conf
    ```
    Add the following lines to the end of the file:
    ```text
    listener 1883
    allow_anonymous false
    password_file /etc/mosquitto/passwd
    ```

3. **Fix Permissions**:
    Ensure the Mosquitto service account can read the password file.
    ```bash
    sudo chown mosquitto:mosquitto /etc/mosquitto/passwd
    sudo chown mosquitto:mosquitto /etc/mosquitto/mosquitto.conf
    ```

4. **Restart Service**:
    ```bash
    sudo systemctl restart mosquitto
    ```

### 2.3 Python Environment Setup
Install the required Python libraries for the bridge script.

```bash
pip3 install paho-mqtt pymongo
```
---

## 3. Middleware Deployment

### Create the Logger Script
Create the Python script that bridges MQTT and MongoDB.

1. **Create File**:
    ```bash
    nano logistics_logger.py
    ```

2. **Paste Code**: (See `Cloud-Backend/logistics_logger.py` for full content). Ensure the credential line matches your setup:
    ```python
    client.username_pw_set("admin", "1234")
    ```

---

## 4. Hardware Configuration (ESP32)

### Arduino IDE Setup
1. **Install Libraries**:
    * `PubSubClient` by Nick O'Leary
    * `DHT sensor library` by Adafruit

2. **Configuration Variables** (`ColdChainTracker.ino`):
    Update the following lines with your specific details:
    ```cpp
    const char* WIFI_SSID = "Your_WiFi_Name";
    const char* WIFI_PASSWORD = "Your_WiFi_Password";
    const char* MQTT_SERVER = "34.xx.xx.xx"; // Your GCP External IP

    // MQTT Credentials
    if (client.connect("ESP32Client", "admin", "1234")) { ... }
    ```

---

## 5. Running & Verification

### Start the Logger Service (GCP)
Run the Python script in the SSH terminal:
```bash
python3 logistics_logger.py
```
*Output should show: `Connected to MQTT Broker (Code: 0)`*

### Verify Data Persistence (MongoDB)
Open a new SSH terminal window and use the MongoDB shell to check stored data.

1. **Enter Mongo Shell**:
    ```bash
    mongosh

    ```

2. **Select Database**:
    ```javascript
    use logistics_db
    ```

3. **Query Data**:
    ```javascript
    db.shipment_logs.find().pretty()
    ```

### Check Service Status (Troubleshooting)
If connection fails, check if Mosquitto is running:

```bash
sudo systemctl status mosquitto
```
Check if port 1883 is listening globally:

```bash

sudo ss -tunlp | grep 1883
```
