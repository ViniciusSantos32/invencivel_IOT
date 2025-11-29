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

// ---------- SERVOS ----------
Servo servo1;
Servo servo2;

#define SERVO1_PIN 12
#define SERVO2_PIN 14

// ---------- ULTRASSÔNICO ----------
#define TRIG_PIN 5
#define ECHO_PIN 18
const int limiteDistancia = 20;

// ---------- LED ----------
#define LED_PIN 27

// ---------- MQTT ----------
WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

// Comando remoto vindo da S2
int comandoS2 = 0;

// ----------- Medir distância -----------
long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duracao == 0) return -1;

  return (duracao * 0.034 / 2);
}

// ----------- CALLBACK MQTT -----------
void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)message[i];

  if (String(topic) == "S3/ComandoMovimento") {
    comandoS2 = msg.toInt();
  }

  if (String(topic) == "S3/Servo1") servo1.write(msg.toInt());
  if (String(topic) == "S3/Servo2") servo2.write(msg.toInt());
}

// ----------- Reconectar MQTT -----------
void reconnectMQTT() {
  while (!mqtt.connected()) {
    if (mqtt.connect("S3_Node", brokerUser, brokerPass)) {

      mqtt.subscribe("S3/Servo1");
      mqtt.subscribe("S3/Servo2");
      mqtt.subscribe("S3/ComandoMovimento"); // comando da S2

    } else {
      delay(5000);
    }
  }
}

// ------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  servo1.attach(SERVO1_PIN, 500, 2400);
  servo2.attach(SERVO2_PIN, 500, 2400);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // WiFi
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  wifi_client.setInsecure();
  mqtt.setServer(brokerURL, brokerPort);
  mqtt.setCallback(callback);
}

// ------------------- LOOP --------------------
void loop() {
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  long distancia = medirDistancia();
  mqtt.publish("S3/Presenca", String(distancia).c_str());

  bool deteccaoLocal = (distancia > 0 && distancia <= limiteDistancia);

  // ATIVA se: S2 mandar 1 OU sensor local detectar
  bool ativar = deteccaoLocal || (comandoS2 == 1);

  if (ativar) {
    servo1.write(120);
    servo2.write(60);
    digitalWrite(LED_PIN, HIGH);
  } else {
    servo1.write(0);
    servo2.write(0);
    digitalWrite(LED_PIN, LOW);
  }

  delay(100);
}
