#include "ArduinoJson.h"
#include "EspMQTTClient.h"

#define measureBtn 22
#define heartRateMeter 33
#define thermometer 32
#define pressureGauge 35
#define MQTT_TOPIC "patientMeasures"


// MQTT Configuracoes
EspMQTTClient client{
  "Wokwi-GUEST", //SSID do WiFi
  "",     // Senha do wifi
  "mqtt.tago.io",  // Endereço do servidor
  "Default",       // Usuario
  "4541e539-018b-4442-861d-890668cf0f6a",         // Token do device
  "esp",           // Nome do device
  1883             // Porta de comunicação
};
// Callback da EspMQTTClient
void onConnectionEstablished()
{}

void setup() {
  Serial.begin(9600);
  pinMode(measureBtn, INPUT);
  pinMode(heartRateMeter, INPUT);
  pinMode(thermometer, INPUT);
  pinMode(pressureGauge, INPUT);
  Serial.println("Conectando WiFi");
  while (!client.isWifiConnected()) {
    Serial.print('.'); client.loop(); delay(1000);
  }
  Serial.println("WiFi Conectado");
  Serial.println("Conectando com Servidor MQTT");
  while (!client.isMqttConnected()) {
    Serial.print('.'); client.loop(); delay(1000);
  }
  Serial.println("MQTT Conectado");
  Serial.println("Sistema conectado, basta apertar o botão para iniciar a medição.");
}

int heartRate = 0;
float temperature = 0.0;
String pressure = "";

void loop() {
  int measureBtnValue = digitalRead(measureBtn);
  if(measureBtnValue == HIGH){
    heartRate = readHeartRate();
    temperature = readTemperature(); 
    pressure = readPressure();

    sendHeartRateToTago();
    sendTemperatureToTago();
    sendPressureToTago();

    delay(500);
  }
  
  client.loop();
}

// Função para mapear um valor de uma faixa para outra
int mapValue(int value, int inMin, int inMax, int outMin, int outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

int readHeartRate(){
  int value = analogRead(heartRateMeter);
  int realValue = mapValue(value, 0, 4095,65, 170);
  return realValue;
}

const float BETA = 3950; // Deve corresponder ao coeficiente beta do termistor

float readTemperature(){
  int analogValue = analogRead(thermometer);
  float celsius = 1 / (log(1 / (4095. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
  return celsius;
}

String readPressure(){
  int value = analogRead(pressureGauge);
  int sistolePressure = mapValue(value, 0, 4095,90, 140);
  int diastolePressure = mapValue(value, 0, 4095,60, 80);
  String result = "" + String(sistolePressure) + "/" + String(diastolePressure);
  return result;
}


char bufferJsonHeart[100];
char bufferJsonTemperature[100];
char bufferJsonPressure[100];

void sendHeartRateToTago(){
  StaticJsonDocument<300> documentoJson;
  documentoJson["variable"] = "HeartRate";
  documentoJson["value"] = heartRate;
  serializeJson(documentoJson, bufferJsonHeart);
  Serial.println(bufferJsonHeart);
  client.publish(MQTT_TOPIC, bufferJsonHeart);
}

void sendTemperatureToTago(){
  StaticJsonDocument<300> documentoJson;
  documentoJson["variable"] = "Temperature";
  documentoJson["value"] = temperature;
  serializeJson(documentoJson, bufferJsonTemperature);
  Serial.println(bufferJsonTemperature);
  client.publish(MQTT_TOPIC, bufferJsonTemperature);
}


void sendPressureToTago(){
  StaticJsonDocument<300> documentoJson;
  documentoJson["variable"] = "Pressure";
  documentoJson["value"] = pressure;
  serializeJson(documentoJson, bufferJsonPressure);
  Serial.println(bufferJsonPressure);
  client.publish(MQTT_TOPIC, bufferJsonPressure);
}