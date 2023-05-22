#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "******"
#define BLYNK_DEVICE_NAME "sequencial de leds"
#define BLYNK_AUTH_TOKEN "*******"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "*******";
char ssid[] = "*******";
char pass[] = "*******";

int LEDVermelho = 32;
int LEDAmarelo = 26;
int LEDVerde = 13;

BLYNK_WRITE (V0){
  int valor = param.asInt();
  digitalWrite(LEDVermelho, valor);
}

BLYNK_WRITE (V1){
  int valor = param.asInt();
  digitalWrite(LEDAmarelo, valor);

}

BLYNK_WRITE (V2){
  int valor = param.asInt();
  digitalWrite(LEDVerde, valor);
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  pinMode(LEDVermelho, OUTPUT);
  pinMode(LEDAmarelo, OUTPUT);
  pinMode(LEDVerde, OUTPUT);
}
2
void loop()
{
  Blynk.run();
}
