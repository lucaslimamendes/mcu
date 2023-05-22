#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN 32
#define PHPIN 35
#define TDSPIN 34
#define VREF 3.3

float phValue;
float tdsValue;
float phRead = 0;
float tdsSensorRead = 0;
float temperature = 0;

DHT dht(DHTPIN, DHTTYPE);

void DHT11SENSOR() {
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
}

void PHSENSOR() {
  phRead = analogRead(PHPIN);
  
  float volt = (float)phRead*5.0/1024/6;
  phValue = 7 + ((2.5 - volt) / 0.18);

  Serial.print("PH: ");
  Serial.println(phValue);
}

void TDSSENSOR() {
  tdsSensorRead = analogRead(TDSPIN);

  float voltage = tdsSensorRead * (float)VREF / 4096.0;
  float compensationCoefficient = 1.0+0.02*(temperature-25.0);
  float compensationVoltage = voltage/compensationCoefficient;
  tdsValue = (133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
  float newTdsValue = tdsValue * 0.0015625;

  Serial.print("TDS: ");
  Serial.print(tdsValue,0);
  Serial.println(" ppm ");
  Serial.print(newTdsValue,0);
  Serial.println(" ms/cm");
}

void setup() {
  Serial.begin(115200);
  pinMode(PHPIN, INPUT);
  pinMode(TDSPIN, INPUT);
  dht.begin();
}

void loop() {
  DHT11SENSOR();
  PHSENSOR();
  TDSSENSOR();

  delay(3000);
}
