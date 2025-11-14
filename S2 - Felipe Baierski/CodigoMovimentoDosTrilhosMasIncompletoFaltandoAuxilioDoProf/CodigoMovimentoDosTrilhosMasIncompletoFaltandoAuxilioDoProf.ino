#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ------------------- WIFI --------------------
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

// ------------------ MQTT ---------------------
const char* brokerURL = "bbfdabd6c614412b9e57017649d99508.s1.eu.hivemq.cloud";
const int brokerPort = 8883;
const char* brokerUser = "baierski_melhor_de_todos";
const char* brokerPass = "Felipe19122007";

// ------------------- PINOS --------------------
#define TRIGGER_PIN 5
#define ECHO_PIN 18

#define SERVO1_PIN 21
#define SERVO2_PIN 19

#define LDR_PIN 36   // A0 (sensor de iluminação)

// ------------------- OBJETOS --------------------
Servo servo1;
Servo servo2;
WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

// ------------------- FUNÇÃO: ULTRASSÔNICO --------------------
long readUltrasonic() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  return distance;
}

// ------------------- CALLBACK MQTT --------------------
void callback(char* topic, byte* message, unsigned int length) {
  String received = "";
  for (unsigned int i = 0; i < length; i++) {
    received += (char)message[i];
  }

  Serial.print("Mensagem recebida em ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(received);

  if (String(topic) == "S3/Servo1") {
    int ang = received.toInt();
    servo1.write(ang);
  }

  if (String(topic) == "S3/Servo2") {
    int ang = received.toInt();
    servo2.write(ang);
  }
}

// ------------------- RECONNECT MQTT --------------------
void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.println("Conectando ao MQTT...");

    if (mqtt.connect("S3_Node", brokerUser, brokerPass)) {
      Serial.println("Conectado!");

      mqtt.subscribe("S3/Servo1");
      mqtt.subscribe("S3/Servo2");

    } else {
      Serial.print("Falha MQTT, rc=");
      Serial.println(mqtt.state());
      delay(1500);
    }
  }
}

// ------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  // Servos
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // Sensor
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(SSID, PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }

  Serial.println("\nWiFi conectado!");

  wifi_client.setInsecure();   // necessário para MQTTs sem certificado
  mqtt.setServer(brokerURL, brokerPort);
  mqtt.setCallback(callback);
}

// ------------------- LOOP --------------------
void loop() {
  if (!mqtt.connected()) {
    reconnectMQTT();
  }
  mqtt.loop();

  long distancia = readUltrasonic();
  int luz = analogRead(LDR_PIN);

  mqtt.publish("S3/Presenca", String(distancia).c_str());
  mqtt.publish("S3/Iluminacao", String(luz).c_str());

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Luz: ");
  Serial.println(luz);

  delay(800);
}
