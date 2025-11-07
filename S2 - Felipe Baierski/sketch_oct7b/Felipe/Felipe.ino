#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure wifi_client;
PubSubClient mqtt(wifi_client);

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* brokerURL = "bbfdabd6c614412b9e57017649d99508.s1.eu.hivemq.cloud";
const int brokerPort = 8883;

const char* topicPub = "testeMensagem";
const char* topicSub = "viniciusGordoNojento";

const char* brokerUser = "baierski_melhor_de_todos"; 
const char* brokerPass = "Felipe19122007";

int pinoPIR = 23;   // Pino do sensor PIR
int pinoLED = 2;    // Pino do LED
int valor = 0;      // Guarda o valor do sensor

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensagem);
}

void setup() {
  Serial.begin(115200);           // Inicia comunicação serial
  WiFi.begin(SSID, PASS);
  Serial.print("conectado com o wifi com sucesso");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println("wifi conectado");

  wifi_client.setInsecure();
  mqtt.setServer(brokerURL, brokerPort);
  mqtt.setCallback(callback);

  Serial.println("conectado ao mqtt");

  pinMode(pinoPIR, INPUT);        // Configura o PIR como entrada
  pinMode(pinoLED, OUTPUT);       // Configura o LED como saída
  Serial.println("Sensor e LED prontos!");

  Serial.print("Conectando ao broker MQTT");
  while (!mqtt.connect("S4-Cliente", brokerUser, brokerPass)) {
    Serial.print(".");
    delay(500);
  }
  mqtt.subscribe(topicSub);
  Serial.println("\nConectado ao HiveMQ Cloud!");
}

void loop() {

  valor = digitalRead(pinoPIR); // Lê o valor do sensor
  if (valor == HIGH) {            // Se detectar movimento
    digitalWrite(pinoLED, HIGH);  // Acende o LED
    enviarParaHiveClound("Movimento detectado!");
  } else {                        // Se não detectar movimento
    digitalWrite(pinoLED, LOW);   // Apaga o LED
    Serial.println("Sem movimento... LED apagado.");
    enviarParaHiveClound("Movimento não detectado!");
  }

  delay(500);
  
   // Espera meio segundo antes de repetir
}

void enviarParaHiveClound(String dado){
 if (!mqtt.connected()) {
    Serial.println("Reconectando ao broker...");
    while (!mqtt.connect("S4-Cliente", brokerUser, brokerPass)) {
      delay(1000);
    }
  }

  mqtt.loop();

  String mensagem = "Felipe dados: " + dado;
  mqtt.publish(topicPub, mensagem.c_str());
}