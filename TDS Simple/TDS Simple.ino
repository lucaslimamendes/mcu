#define TdsSensorPin 27
#define VREF 3.3              // analog reference voltage(Volt) of the ADC

float voltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

void setup() {
  Serial.begin(115200);
  pinMode(TdsSensorPin,INPUT);  
}

void loop() {
    float tdsSensorRead = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    voltage = tdsSensorRead * (float)VREF / 4096.0;

    float compensationCoefficient = 1.0+0.02*(temperature-25.0);
    //temperature compensation
    float compensationVoltage = voltage/compensationCoefficient;
    //convert voltage value to tds value
    tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;

    Serial.print("TDS Value: ");
    Serial.print(tdsValue,0);
    Serial.println(" ppm");
      
    delay(3000);
}
