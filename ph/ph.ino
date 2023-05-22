const int potPin = 35;
float ph;
float Value = 0;

void setup() {
  Serial.begin(115200);
  pinMode(potPin,INPUT);
  delay(2000);
}

void loop() {
  Value = analogRead(potPin);
  Serial.print(Value);
  Serial.print(" | ");

  // float volt = (float)Value/6*3.4469/4095;
  // ph = -5.70 * volt + 21.34;

  // float volt = (float)Value*3.3/1024/6;
  // ph = 3.5*volt;    
  
  float volt = (float)Value*5.0/1024/6;
  ph = 7 + ((2.5 - volt) / 0.18);

  Serial.println(ph);

  delay(2000);
}
