#include <PubSubClient.h> 
#include <WiFi.h> 
#include "DHT.h" 
#define DHTTYPE DHT11 
 
const char* WIFI_SSID = "TIMI";           // Your WiFi SSID 
const char* WIFI_PASSWORD = "985usm211";   // Your WiFi password 
const char* MQTT_SERVER = "34.42.105.151";  // Your VM instance public IP address 
const char* MQTT_TOPIC = "coldchain/telemetry";           // MQTT topic for subscription 
const int MQTT_PORT = 1883;               // Non-TLS communication port 
const int dht11Pin = 42;                  // DHT11 sensor pin 
char buffer[128] = "";                    // Text buffer 
DHT dht(dht11Pin, DHTTYPE); 
WiFiClient espClient; 
PubSubClient client(espClient); 
 
void setup_wifi() { 
  delay(10); 
  Serial.println(); 
  Serial.print("Connecting to "); 
  Serial.println(WIFI_SSID); 
 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
 
  Serial.println(""); 
  Serial.println("WiFi connected"); 
  Serial.print("IP address: "); 
  Serial.println(WiFi.localIP()); 
} 
 
void setup() { 
  Serial.begin(115200);                     // Initiate serial communication 
  dht.begin();                              // Initialize DHT sensor 
  setup_wifi();                             // Connect to the WiFi network 
  client.setServer(MQTT_SERVER, MQTT_PORT); // Set up the MQTT client 
} 
 

void loop() {
  if(!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(5000); // 每5秒上报一次

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // --- 核心逻辑修改开始 ---
  
  // 1. 定义合规阈值 (例如 25度)
  const float TEMP_THRESHOLD = 25.0; 
  const char* status = "OK";

  // 2. 边缘计算 (Edge Computing): 判断是否超温
  if (temperature > TEMP_THRESHOLD) {
    status = "BREACH"; // 温度过高，标记违规
  }

  // 3. 构建 JSON 字符串
  // 模拟一个运输批次号 "TRIP-2026-A01"
  // 格式: {"trip_id": "...", "temp": 24.5, "humid": 60, "status": "OK"}
  sprintf(buffer, "{\"trip_id\": \"TRIP-2026-A01\", \"temp\": %.2f, \"humid\": %.2f, \"status\": \"%s\"}", temperature, humidity, status);

  // 4. 发布到 GCP MQTT
  client.publish("coldchain/telemetry", buffer); 
  Serial.print("Data Published: ");
  Serial.println(buffer);
  

}

void reconnect() {
  // 循环直到连接成功
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // 尝试连接，客户端ID设为 "ESP32Client"
    if (client.connect("ESP32Client", "admin", "1234")) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}