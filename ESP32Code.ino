#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiClientSecure.h>

// ===== WiFi Settings =====
const char* ssid = "------------";
const char* password = "-----------";

// ===== EMQX MQTT Settings =====
const char* mqtt_server = "-----------------";
const int mqtt_port = 8883;
const char* mqtt_topic = "stress_detection/subject01";
const char* mqtt_result_topic = "stress_detection/results";
const char* mqtt_user = "---------";
const char* mqtt_password = "----------";

// ===== Certificate (EMQX TLS root CA) =====
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

// ===== Sensor Settings =====
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define GSR_PIN 35
#define PULSE_PIN 34

// ===== RGB LED Common Anode Pins =====
#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27

void setColor(bool red, bool green, bool blue) {
  // Common Anode LED: LOW → burns, HIGH → goes out
  digitalWrite(RED_PIN, red ? LOW : HIGH);
  digitalWrite(GREEN_PIN, green ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blue ? LOW : HIGH);
}

// ===== Structures and MQTT Settings =====
WiFiClientSecure espClient;
PubSubClient client(espClient);

struct SensorData {
  float temperature;
  float humidity;
  int gsr;
  int pulse;
};

SensorData sensorData[60];
int sampleIndex = 0;
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 1000; // 1 second

// ===== WiFi Connect =====
void setup_wifi() {
  Serial.print("Connecting WiFi ...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected. IP: " + WiFi.localIP().toString());
}

// ===== MQTT Callback =====
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim();
  Serial.println("📩 Incoming predict: " + msg);

  // Extract prediction value from JSON
  int startIndex = msg.indexOf("\"prediction\":");
  if (startIndex != -1) {
    startIndex = msg.indexOf("\"", startIndex + 13) + 1;
    int endIndex = msg.indexOf("\"", startIndex);
    String prediction = msg.substring(startIndex, endIndex);

    Serial.println("🔍 Predict: " + prediction);

    if (prediction == "rahat") {
      setColor(false, true, false);  // green
    } else if (prediction == "stresli") {
      setColor(false, false, true);  // blue
    } else if (prediction == "cok stresli") {
      setColor(true, false, false);  // red
    } else {
      setColor(false, false, false); // close
    }

    delay(5000);
    setColor(false, false, false);
  } else {
    Serial.println("⚠️ prediction field not found.");
  }
}

// ===== MQTT Connect Function =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT connecting...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("✅ MQTT connected.");
      client.subscribe(mqtt_result_topic);
    } else {
      Serial.print("❌ Connection failed. Retrying...");
      delay(2000);
    }
  }
}

// ===== JSON Creating a Package =====
String createFullPayload() {
  String payload = "{\"timestamp\":" + String(millis()) + ",\"samples\":[";
  for (int i = 0; i < 60; i++) {
    payload += "{";
    payload += "\"temperature\":" + String(sensorData[i].temperature, 1) + ",";
    payload += "\"humidity\":" + String(sensorData[i].humidity, 1) + ",";
    payload += "\"gsr\":" + String(sensorData[i].gsr) + ",";
    payload += "\"pulse\":" + String(sensorData[i].pulse) + "}";
    if (i < 59) payload += ",";
  }
  payload += "]}";
  return payload;
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(GSR_PIN, INPUT);
  pinMode(PULSE_PIN, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setColor(false, false, false);

  setup_wifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(4096);
}

// ===== Main Loop =====
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long currentMillis = millis();

  if ((currentMillis - lastSampleTime >= sampleInterval) && sampleIndex < 60) {
    lastSampleTime = currentMillis;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    sensorData[sampleIndex].temperature = isnan(temp) ? -1.0 : temp;
    sensorData[sampleIndex].humidity = isnan(hum) ? -1.0 : hum;
    sensorData[sampleIndex].gsr = analogRead(GSR_PIN);
    sensorData[sampleIndex].pulse = analogRead(PULSE_PIN);

    sampleIndex++;
  }

  if (sampleIndex >= 60) {
    String payload = createFullPayload();
    Serial.println("📤1 minute data is sent via MQTT...");
    if (client.publish(mqtt_topic, (const uint8_t*)payload.c_str(), payload.length(), false)) {
      Serial.println("✅ Sending successful.");
    } else {
      Serial.println("❌ Send failed.");
    }

    sampleIndex = 0;
    delay(10000); // wait 10 second
  }
}

