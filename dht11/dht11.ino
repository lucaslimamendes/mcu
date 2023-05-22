#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "*****" // mostrado no site
#define BLYNK_DEVICE_NAME "Temperatura e Umidade"
// Biblioteca necessárias
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char auth[] = "****"; // Aqui é necessário inserir o token exibido no site
char ssid[] = "****"; // Insira o nome da rede Wi-fi utilizada
char pass[] = "****"; // Insira a senha da rede Wi-fi utilizada

#define DHTPIN 2 // Aqui é o pino digital que estamos utilizando, no nosso caso D2 (GPIO 2)
#define DHTTYPE DHT11 // Declarando o sensor

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, h); // Pino Virtual 0 para umidade
  Blynk.virtualWrite(V6, t); // Pino Virtual 6 para temperatura
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
}
