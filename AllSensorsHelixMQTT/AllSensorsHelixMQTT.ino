#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

#define DHTTYPE DHT11
#define DHTPIN 32
#define PHPIN 35
#define TDSPIN 34
#define VREF 3.3

#define TOPICO_PUBLISH_PH   "/iot/ph001/attrs/ph"
#define TOPICO_PUBLISH_TDS   "/iot/tds001/attrs/tds"
#define TOPICO_PUBLISH_DHT11_T   "/iot/dht11001/attrs/t"
#define TOPICO_PUBLISH_DHT11_H   "/iot/dht11001/attrs/h"

#define ID_MQTT  "helix_first_esp"

// WiFi Network Configuration
const char *ssid = "****";
const char *password = "****";
const char *hostname = "ESP32";

// MQTT
const char* BROKER_MQTT = "****"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

float phValue;
float tdsValue;
float phRead = 0;
float tdsSensorRead = 0;
float temperature = 0;

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

DHT dht(DHTPIN, DHTTYPE);

void initWifi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(50);
  }
  Serial.print("\n IP: " + WiFi.localIP());
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
  String msg;
  for(int i = 0; i < length; i++) 
  {
    char c = (char)payload[i];
    msg += c;
  }
  
  Serial.print("mqtt_callback MSG: ");
  Serial.println(msg);
}

void initMQTT() 
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);  //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

void reconnectMQTT() 
{
  while (!MQTT.connected()) 
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) 
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
    } 
    else
    {
      Serial.println("Falha ao reconectar no broker. Havera nova tentatica de conexao em 2s.");
      delay(2000);
    }
  }
}

void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected()) 
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
    
  initWifi(); //se não há conexão com o WiFI, a conexão é refeita
}

void DHT11SENSOR() {
  char buffer[20];
  char buffer2[20];
  float h = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(h) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Serial.print("Temperatura: ");
  Serial.println(temperature);
  Serial.print("Humidade: ");
  Serial.println(h);

  MQTT.publish(TOPICO_PUBLISH_DHT11_T, dtostrf(temperature, 2, 2, buffer));
  MQTT.publish(TOPICO_PUBLISH_DHT11_H, dtostrf(h, 2, 2, buffer2));
}

void PHSENSOR() {
  char buffer[20];
  phRead = analogRead(PHPIN);
  
  float volt = (float)phRead*5.0/1024/6;
  phValue = 7 + ((2.5 - volt) / 0.18);

  Serial.print("PH: ");
  Serial.println(phValue);
  MQTT.publish(TOPICO_PUBLISH_PH, dtostrf(phValue, 2, 2, buffer));
}

void TDSSENSOR() {
  char buffer[20];
  tdsSensorRead = analogRead(TDSPIN);

  float voltage = tdsSensorRead * (float)VREF / 4096.0;
  float compensationCoefficient = 1.0+0.02*(temperature-25.0);
  float compensationVoltage = voltage/compensationCoefficient;
  tdsValue = (133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
  
  Serial.print("TDS: ");
  Serial.print(tdsValue,0);
  Serial.println(" ppm");
  MQTT.publish(TOPICO_PUBLISH_TDS, dtostrf(tdsValue, 2, 2, buffer));
}

void setup() {
  Serial.begin(115200);
  pinMode(PHPIN, INPUT);
  pinMode(TDSPIN, INPUT);
  dht.begin();
  initWifi();
  initMQTT();
  delay(5000);
}

void loop() {
  VerificaConexoesWiFIEMQTT();

  DHT11SENSOR();
  delay(1500);

  TDSSENSOR();
  delay(1500);

  PHSENSOR();
  delay(1500);
  
  MQTT.loop();
}
