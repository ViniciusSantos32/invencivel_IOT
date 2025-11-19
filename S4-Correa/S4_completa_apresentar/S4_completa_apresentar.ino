#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* brokerURL = "bbfdabd6c614412b9e57017649d99508.s1.eu.hivemq.cloud";
const int brokerPort = 8883;
const char* brokerUser = "baierski_melhor_de_todos";
const char* brokerPass = "Felipe19122007";

const char* topic_command = "mariah/iot/sinal";

#define LED_VERDE    23
#define LED_VERMELHO 22

const unsigned long TIMEOUT_SEM_MENSAGEM = 2000;
unsigned long ultimoMomentoMensagem = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("Mensagem recebida: ");
  Serial.println(msg);

  ultimoMomentoMensagem = millis();

  // LEDs ativos em LOW → inverter HIGH/LOW
  digitalWrite(LED_VERMELHO, HIGH); // APAGA vermelho
  digitalWrite(LED_VERDE, LOW);     // ACENDE verde
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32", brokerUser, brokerPass)) {
      Serial.println("Conectado!");
      client.subscribe(topic_command);
    } else {
      Serial.println("Falhou, tentando novamente...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  // Estado inicial: VERDE apagado, VERMELHO aceso
  digitalWrite(LED_VERDE, HIGH);     // APAGA verde
  digitalWrite(LED_VERMELHO, LOW);   // ACENDE vermelho

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  espClient.setInsecure();
  client.setServer(brokerURL, brokerPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) mqttReconnect();
  client.loop();

  if (millis() - ultimoMomentoMensagem > TIMEOUT_SEM_MENSAGEM) {
    // Sem mensagem → LED vermelho ACESO
    digitalWrite(LED_VERDE, HIGH);    // Apaga verde
    digitalWrite(LED_VERMELHO, LOW);  // Acende vermelho
  }
}
