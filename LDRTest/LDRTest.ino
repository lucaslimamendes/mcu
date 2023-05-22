#include <WiFi.h>
#include <PubSubClient.h>

#define TOPICO_SUBSCRIBE "/iot/lamp001/cmd"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "/iot/lamp001/attrs"    //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_2   "/iot/lamp001/attrs/l" //tópico MQTT de envio de informações para Broker

#define ID_MQTT  "helixLDRTest"

#define Light 34
#define Led 15

const char *ssid = "*****";
const char *password = "*****";
const char *hostname = "ESP32";

const char* BROKER_MQTT = "192.168.0.23"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQT

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
char EstadoSaida = '1';  //variável que armazena o estado atual da saída

void setup() {
    Serial.begin(115200);
    Serial.println("\nBootting...");
    InitOutput();
    setupWiFi();
    initMQTT();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH, "s|off");
}

void setupWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(50);
    }
    Serial.print("\n IP: " + WiFi.localIP());
    Serial.println(WiFi.localIP());
}

void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //toma ação dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída D0:
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    if (msg.equals("lamp001@on|"))
    {
        digitalWrite(Led, LOW);
        EstadoSaida = '0';
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("lamp001@off|"))
    {
        digitalWrite(Led, HIGH);
        EstadoSaida = '1';
    }
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
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.setHostname(hostname);
    WiFi.begin(ssid, password);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(ssid);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void EnviaEstadoOutputMQTT(void)
{
    if (EstadoSaida == '0')
      MQTT.publish(TOPICO_PUBLISH, "s|on");
 
    if (EstadoSaida == '1')
      MQTT.publish(TOPICO_PUBLISH, "s|off");
 
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}

void InitOutput(void)
{
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    pinMode(Led, OUTPUT);
    digitalWrite(Led, HIGH);          
}  

void loop() {
    char msgBuffer[1];
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();

    //envia o status de todos os outputs para o Broker no protocolo esperado
    EnviaEstadoOutputMQTT();

    //luminosidade
    int sensorValue = analogRead(Light);   // Ler o pino Analógico A0 onde está o LDR
    float voltage = sensorValue * (3.3 / 1024.0);   // Converter a leitura analógica (que vai de 0 - 1023) para uma voltagem (0 - 3.3V), quanto de acordo com a intensidade de luz no LDR a voltagem diminui.
    Serial.println("sensorValue ");
    Serial.println(sensorValue);
    Serial.println("voltage ");
    Serial.println(voltage);
    MQTT.publish(TOPICO_PUBLISH_2,dtostrf(voltage, 4, 2, msgBuffer));
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();

    delay(5000);
}
