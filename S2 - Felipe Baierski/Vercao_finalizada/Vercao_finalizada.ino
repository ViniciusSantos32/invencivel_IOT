#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ------------------- WIFI --------------------
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

// ------------------ MQTT ---------------------
const char* brokerURL = "bbfdabd6c614412b9e57017649d99508.s1.eu.hivemq.cloud";
const int brokerPort = 8883;
const char* brokerUser = "baierski_melhor_de_todos";
const char* brokerPass = "Felipe19122007";

// ---------- PINOS SENSORES ----------
const int TRIG_PIN_S1 = 12;
const int ECHO_PIN_S1 = 14;
const int TRIG_PIN_S2 = 27;
const int ECHO_PIN_S2 = 26;

// ---------- LED (opcional) ----------
const int LED2 = 5;

// Distância máxima (cm)
const int DISTANCIA_MAXIMA = 20;

// MQTT
WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

// ----------- Função para medir distância -----------
long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 25000);

  if (duracao == 0) return -1;

  return (duracao * 0.034 / 2);
}

// ----------- Reconexão MQTT -----------
void reconnectMQTT() {
  while (!mqtt.connected()) {
    if (mqtt.connect("S2_Node", brokerUser, brokerPass)) {
      // S2 só publica
    } else {
      delay(3000);
    }
  }
}

// ------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  wifi_client.setInsecure();

  // Pinos sensores
  pinMode(TRIG_PIN_S1, OUTPUT);
  pinMode(ECHO_PIN_S1, INPUT);

  pinMode(TRIG_PIN_S2, OUTPUT);
  pinMode(ECHO_PIN_S2, INPUT);

  pinMode(LED2, OUTPUT);

  // WiFi
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  mqtt.setServer(brokerURL, brokerPort);

  Serial.println("S2 pronta para enviar detecção via MQTT...");
}

// ------------------- LOOP --------------------
void loop() {
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  long d1 = medirDistancia(TRIG_PIN_S1, ECHO_PIN_S1);
  long d2 = medirDistancia(TRIG_PIN_S2, ECHO_PIN_S2);

  bool detecao = ((d1 > 0 && d1 <= DISTANCIA_MAXIMA) ||
                  (d2 > 0 && d2 <= DISTANCIA_MAXIMA));

  // Envia comando para S3
  if (detecao) {
    mqtt.publish("S3/ComandoMovimento", "1");
    digitalWrite(LED2, HIGH);
    Serial.println("OBJETO DETECTADO (S2) → enviando 1");
  } else {
    mqtt.publish("S3/ComandoMovimento", "0");
    digitalWrite(LED2, LOW);
    Serial.println("Nada detectado (S2) → enviando 0");
  }

  delay(400);
}