#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifi_client; //nome do cliente wifi
PubSubClient mqtt(wifi_client);

const String SSID = "FIESC_IOT_EDU"; //nome da rede wifi
const String PASS = "8120gv08";//senha da rede

const String brokerURL = "teste.mosquitto.org";
const int brokerPort = 1883;

const String brokerUser = "";
const String brokerPass = "";

void setup() {
  Serial.begin(115200); //configura a placa para mostrar na tela
  WiFi.begin(SSID, PASS); //tenta conectar na rede
  Serial.println("Conectando no WiFi");
  while (WiFi.status()!= WL_CONNECTED){
     Serial.print(".");
     delay(200);
  }
  Serial.println("Conectado com sucesso!");
  mqtt.setServer(brokerURL.c_str(),brokerPort);
  String clientID = "S1-";
  clientID += String(random(0xffff),HEX);
  Serial.println("Conectado ao Broker");
  while(mqtt.connect(clientID.c_str()) == 0){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado ao Broker!");
}



void loop() {
  // put your main code here, to run repeatedly:

}