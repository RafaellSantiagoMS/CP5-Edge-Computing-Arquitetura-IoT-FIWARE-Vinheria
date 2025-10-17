#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======= CONFIGURAÇÕES EDITÁVEIS =======
const char* SSID = "Wokwi-GUEST";          // Wi-Fi SSID
const char* PASSWORD = "";                  // Wi-Fi Senha
const char* BROKER_MQTT = "54.233.36.41"; // IP público da sua instância EC2
const int BROKER_PORT = 1883;               // Porta do broker MQTT
const char* TOPICO_SUBSCRIBE = "/TEF/lamp001/cmd";
const char* TOPICO_PUBLISH_1 = "/TEF/lamp001/attrs";
const char* TOPICO_PUBLISH_2 = "/TEF/lamp001/attrs/l";
const char* TOPICO_PUBLISH_3 = "/TEF/lamp001/attrs/h";
const char* TOPICO_PUBLISH_4 = "/TEF/lamp001/attrs/t";
const char* TOPICO_PUBLISH_5 = "/TEF/lamp001/attrs/alagamento";
const char* ID_MQTT = "fiware_001";
const int LED_PIN = 2; // LED onboard do ESP32

// ======= SENSOR DHT =======
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ======= OBJETOS WIFI E MQTT =======
WiFiClient espClient;
PubSubClient MQTT(espClient);

// ======= VARIÁVEIS GLOBAIS =======
char EstadoSaida = '0';
unsigned long lastMsg = 0; // Controle de tempo (para evitar delays)


// ======= INICIALIZAÇÕES =======
void initSerial() {
  Serial.begin(115200);
  Serial.println("Inicializando Serial...");
}

void initWiFi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

// ======= CALLBACK MQTT =======
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("Comando recebido em ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  if (msg.equalsIgnoreCase("on")) {
    digitalWrite(LED_PIN, HIGH);
    EstadoSaida = '1';
  } else if (msg.equalsIgnoreCase("off")) {
    digitalWrite(LED_PIN, LOW);
    EstadoSaida = '0';
  }
}

// ======= CONEXÕES =======
void reconectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconectando Wi-Fi...");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\nReconectado ao Wi-Fi!");
  }
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Conectando ao broker MQTT em ");
    Serial.print(BROKER_MQTT);
    Serial.println(" ...");
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha na conexão. Tentando novamente em 2s...");
      delay(2000);
    }
  }
}

void VerificaConexoesWiFIEMQTT() {
  reconectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}
