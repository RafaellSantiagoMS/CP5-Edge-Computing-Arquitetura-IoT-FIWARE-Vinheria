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

// ======= PUBLICAÇÕES =======
void EnviaEstadoOutputMQTT() {
  if (EstadoSaida == '1') {
    MQTT.publish(TOPICO_PUBLISH_1, "s|on");
    Serial.println("LED ligado (enviado ao broker)");
  } else {
    MQTT.publish(TOPICO_PUBLISH_1, "s|off");
    Serial.println("LED desligado (enviado ao broker)");
  }
}

void handleSensores() {
  // Luminosidade (potenciômetro simulando)
  int potLum = analogRead(34);
  int luminosity = map(potLum, 0, 4095, 0, 100);
  MQTT.publish(TOPICO_PUBLISH_L, String("l|" + String(luminosity)).c_str());
  Serial.printf("Luminosidade: %d\n", luminosity);

  // Temperatura e Umidade
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    MQTT.publish(TOPICO_PUBLISH_H, String("h|" + String(h)).c_str());
    MQTT.publish(TOPICO_PUBLISH_T, String("t|" + String(t)).c_str());
    Serial.printf("Umidade: %.2f %% | Temperatura: %.2f °C\n", h, t);
  } else {
    Serial.println("Erro ao ler DHT22!");
  }

  // Alagamento (potenciômetro simulando)
  int potAlag = analogRead(35);
  int alag = map(potAlag, 0, 4095, 0, 100);
  MQTT.publish(TOPICO_PUBLISH_ALAG, String("alagamento|" + String(alag)).c_str());
  Serial.printf("Alagamento: %d\n", alag);
}

// ======= INICIALIZAÇÃO DO LED =======
void InitOutput() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}