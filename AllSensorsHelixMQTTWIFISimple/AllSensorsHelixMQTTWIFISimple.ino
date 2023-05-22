#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD_I2C.h>

#define CONNECTION_TIMEOUT 10
#define TEMPPIN 4
#define PHPIN 35
#define TDSPIN 34
#define VREF 3.3
#define LEDVermelho 32
#define LEDAmarelo 26
#define LEDVerde 13

#define TOPICO_PUBLISH_PH   "/iot/ph001/attrs/ph"
#define TOPICO_PUBLISH_TDS   "/iot/tds001/attrs/tds"
#define TOPICO_PUBLISH_TEMP   "/iot/temp001/attrs/temperature"

#define ID_MQTT  "helix_id_mqtt_esp"

// WiFi Network Configuration
const char *ssid = "****";
const char *password = "****";
const char *hostname = "ESP3232";

// MQTT
const char* BROKER_MQTT = "****"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

int lcdColumns = 16;
int lcdRows = 2;

float phValue;
float tdsValue;
float phRead = 0;
float tdsSensorRead = 0;
float temperature = 0;

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

OneWire oneWire(TEMPPIN);
DallasTemperature sensors(&oneWire);

LCD_I2C lcd(0x27, lcdColumns, lcdRows);

void initWifi()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando...");
  lcd.setCursor(0,1);
  lcd.print("WIFI!");

  digitalWrite(LEDVermelho, HIGH);
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LEDVermelho, LOW);
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);
  int timeout_counter = 0;

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(LEDVermelho, HIGH);
    delay(2000);

    timeout_counter++;
    if(timeout_counter >= CONNECTION_TIMEOUT){
      ESP.restart();
    }
  }
  Serial.print("[WIFI] Connected: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LEDVermelho, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado com");
  lcd.setCursor(0,1);
  lcd.print("sucesso!");
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando...");
    lcd.setCursor(0,1);
    lcd.print("MQTT!");

    digitalWrite(LEDVerde, LOW);
    digitalWrite(LEDAmarelo, HIGH);
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) 
    {
      digitalWrite(LEDAmarelo, LOW);
      Serial.println("Conectado com sucesso ao broker MQTT!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conectado ao");
      lcd.setCursor(0,1);
      lcd.print("broker MQTT!");
    } 
    else
    {
      WiFi.begin(ssid, password);
      Serial.println("Falha ao reconectar no broker. Havera nova tentatica de conexao em 2s.");
      delay(2000);
    }
  }
}

void VerificaConexoesWiFIEMQTT(void)
{
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LEDVerde, LOW);
    digitalWrite(LEDVermelho, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando...");
    lcd.setCursor(0,1);
    lcd.print("WIFI!");
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname);
    WiFi.begin(ssid, password);
  }
  digitalWrite(LEDVermelho, LOW);

  if (!MQTT.connected()) 
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  digitalWrite(LEDAmarelo, LOW);
  digitalWrite(LEDVerde, HIGH);
}

void TEMPSENSOR() {
  char buffer[20];
  sensors.requestTemperatures(); 
  temperature = sensors.getTempCByIndex(0);
  
  Serial.print("Temperatura: ");
  Serial.println(temperature);

  VerificaConexoesWiFIEMQTT();

  MQTT.publish(TOPICO_PUBLISH_TEMP, dtostrf(temperature, 2, 2, buffer));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Valor atual");
  lcd.setCursor(0,1);
  lcd.print("Temp. " + String(temperature) + " C");
}

void PHSENSOR() {
  char buffer[20];
  phRead = analogRead(PHPIN);
  
  float volt = (float)phRead*5.0/1024/6;
  phValue = 7 + ((2.5 - volt) / 0.18);

  Serial.print("PH: ");
  Serial.println(phValue);

  VerificaConexoesWiFIEMQTT();

  MQTT.publish(TOPICO_PUBLISH_PH, dtostrf(phValue, 2, 2, buffer));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Valor atual");
  lcd.setCursor(0,1);
  lcd.print("PH: " + String(phValue));
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

  VerificaConexoesWiFIEMQTT();

  MQTT.publish(TOPICO_PUBLISH_TDS, dtostrf(tdsValue, 2, 2, buffer));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Valor atual");
  lcd.setCursor(0,1);
  lcd.print("TDS: " + String(tdsValue) + " ppm");
}

void setup() {
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Planta&Hidro!");
  delay(2000);

  pinMode(PHPIN, INPUT);
  pinMode(TDSPIN, INPUT);
  pinMode(LEDVermelho, OUTPUT);
  pinMode(LEDAmarelo, OUTPUT);
  pinMode(LEDVerde, OUTPUT);
  sensors.begin();
  initWifi();
  initMQTT();
  delay(2000);
}

void loop() {
  TEMPSENSOR();
  delay(5000);

  TDSSENSOR();
  delay(5000);

  PHSENSOR();
  delay(5000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PH " + String(phValue) + " & " + String(temperature) + "C");
  lcd.setCursor(0,1);
  lcd.print("TDS " + String(tdsValue) + " PPM");

  // delay(10000);
  delay(360000);
}
