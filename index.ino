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
