#include "esp_wpa2.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

#define EAP_IDENTITY "*****"
#define EAP_PASSWORD "*****"

#define ID_MQTT  "helix_qwertdkfjskldfjsy_esp"

const char *ssid = "******";
// MQTT
const char* BROKER_MQTT = "******"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

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

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println(ssid);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
  esp_wifi_sta_wpa2_ent_enable(&config);

  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  HTTPClient http;
  String serverPath = "http://ip-api.com/line";
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  String payload = http.getString();
  Serial.println(payload);
}

void loop()
{
  WiFiClient espClient; // Cria o objeto espClient
  PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
  
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);  //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
  MQTT.connect(ID_MQTT);

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
