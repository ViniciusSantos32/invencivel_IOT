// ========================= BIBLIOTECAS =========================
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// ========================= DHT =========================
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ========================= OBJETOS =========================
WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

// ========================= CREDENCIAIS WI-FI =========================
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

// ========================= CREDENCIAIS MQTT =========================
const char* brokerURL = "bbfdabd6c614412b9e57017649d99508.s1.eu.hivemq.cloud";
const int brokerPort = 8883;
const char* brokerUser = "baierski_melhor_de_todos";
const char* brokerPass = "Felipe19122007";

// Tópicos MQTT
const char* topicUltrassom = "iot/sensor/ultrassonico";
const char* topicTemp = "iot/sensor/temperatura";
const char* topicUmid = "iot/sensor/umidade";
const char* topicJSON = "iot/sensor/dht_json";

// ========================= PINOS =========================
int pinoTrigger = 22;
int pinoEcho = 23;
int pinoLED = 19;
const byte LDR_PIN = 34;

// ========================= VARIÁVEIS =========================
float distancia;
long duracao;


// ========================= FUNÇÕES =========================

// ---- Enviar ao HiveMQ ----
void enviarParaHiveCloud(String mensagem, const char* topico) {
  if (!mqtt.connected()) {
    Serial.println("Reconectando ao broker MQTT...");
    while (!mqtt.connect("ESP32-SENSOR", brokerUser, brokerPass)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println("\nReconectado!");
  }

  mqtt.loop();
  mqtt.publish(topico, mensagem.c_str());
  Serial.print("Publicado em ");
  Serial.print(topico);
  Serial.print(": ");
  Serial.println(mensagem);
}


// ---- Verificar se o DHT está conectado ----
bool verificarDHT() {
  Serial.println("Verificando DHT11...");

  for (int i = 0; i < 10; i++) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      Serial.println("DHT11 OK! Sensor funcionando.");
      return true;
    }

    Serial.println("Tentando detectar o DHT11...");
    delay(500);
  }

  Serial.println("ERRO: DHT11 não encontrado! Verifique fios e conexões.");
  return false;
}



// ========================= SETUP =========================
void setup() {
  Serial.begin(115200);

  pinMode(pinoTrigger, OUTPUT);
  pinMode(pinoEcho, INPUT);
  pinMode(pinoLED, OUTPUT);

  Serial.println("Conectando ao WiFi...");
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado!");

  wifi_client.setInsecure();
  mqtt.setServer(brokerURL, brokerPort);

  Serial.print("Conectando ao HiveMQ...");
  while (!mqtt.connect("ESP32-SENSOR", brokerUser, brokerPass)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConectado ao HiveMQ!");

  dht.begin();

  // ---- Chama o verificador ----
  if (!verificarDHT()) {
    while (true) {
      Serial.println("Corrija o DHT11 e reinicie o ESP32...");
      delay(2000);
    }
  }
}



// ========================= LOOP =========================
void loop() {

  // --- ULTRASSÔNICO ---
  digitalWrite(pinoTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrigger, LOW);

  duracao = pulseIn(pinoEcho, HIGH);
  distancia = duracao * 0.034 / 2;

  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia < 20) {
    digitalWrite(pinoLED, HIGH);
    enviarParaHiveCloud("OBJETO_DETECTADO", topicUltrassom);
  } else {
    digitalWrite(pinoLED, LOW);
    enviarParaHiveCloud("SEM_OBJETO", topicUltrassom);
  }


  // --- LEITURA DO DHT ---
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Erro ao ler o DHT11!");
  } else {
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print("%  | Temperatura: ");
    Serial.print(temperatura);
    Serial.println("°C");

    enviarParaHiveCloud(String(umidade), topicUmid);
    enviarParaHiveCloud(String(temperatura), topicTemp);

    String json = "{ \"temperatura\": " + String(temperatura) +
                  ", \"umidade\": " + String(umidade) + " }";

    enviarParaHiveCloud(json, topicJSON);
  }
  int leituraLDR = analogRead(LDR_PIN);
  float tensao = (leituraLDR * 3.3) / 4095.0;
  
  Serial.print("Leitura LDR: ");
  Serial.print(leituraLDR);
  Serial.print(" - Tensão: ");
  Serial.println(tensao);
  
  if (leituraLDR < 1000) {
    Serial.println("Ambiente escuro");
  } else {
    Serial.println("Ambiente claro");
  }

  delay(3000);

}
