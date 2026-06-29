#include <WiFi.h>
#include "DHTesp.h"
#include "ThingSpeak.h"
// Pines sensores
#define DHTPIN 15
#define MQ2PIN 34
#define PIRPIN 27
// Actuadores PIR
#define LED_JARDIN 2
#define BUZZER 4
// LED RGB Living
#define RED_PIN 18
#define GREEN_PIN 19
#define BLUE_PIN 21
// WiFi
const char* WIFI_NAME = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
// ThingSpeak
unsigned long myChannelNumber = 2269662;
const char* myApiKey = "8H1QS250GQLDN8QH";
WiFiClient client;
DHTesp dhtSensor;
void setup() {
  Serial.begin(115200);
  // Inicializar sensores
  dhtSensor.setup(DHTPIN, DHTesp::DHT22);
  pinMode(PIRPIN, INPUT);
  // Inicializar actuadores
  pinMode(LED_JARDIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  // Conexión WiFi
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi no conectado");
  }
  Serial.println("¡WiFi conectado!");
  ThingSpeak.begin(client);
}
void loop() {
  // Lectura de sensores
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  int gasValue = analogRead(MQ2PIN);
  int pirValue = digitalRead(PIRPIN);
  // Lógica PIR (patio/jardín)
  if (pirValue == HIGH) {
    digitalWrite(LED_JARDIN, HIGH);
    digitalWrite(BUZZER, HIGH);
    Serial.println("⚠️ Movimiento detectado en el patio!");
  } else {
    digitalWrite(LED_JARDIN, LOW);
    digitalWrite(BUZZER, LOW);
  }
  // Lógica LED RGB Living (estado del gas)
  if (gasValue > 3000) {
    // Peligro → rojo
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
  } else {
    // Seguro → verde
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
  }
  // Mostrar datos en monitor serie
  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humedad: " + String(data.humidity, 1) + "%");
  Serial.println("Gas: " + String(gasValue));
  Serial.println("Movimiento PIR: " + String(pirValue));
  Serial.println("LED Jardín: " + String(digitalRead(LED_JARDIN)));
  Serial.println("Living RGB (Rojo/Verde): " + String(digitalRead(RED_PIN)) + "/" + String(digitalRead(GREEN_PIN)));
  Serial.println("---");
  // Enviar datos a ThingSpeak
  ThingSpeak.setField(1, data.temperature);   // DHT22 Temp
  ThingSpeak.setField(2, data.humidity);      // DHT22 Humedad
  ThingSpeak.setField(3, gasValue);           // MQ-2 Gas
  ThingSpeak.setField(4, pirValue);           // Sensor PIR
  ThingSpeak.setField(5, digitalRead(LED_JARDIN)); // LED Jardín
  ThingSpeak.setField(6, digitalRead(RED_PIN));    // Living (Rojo)
  ThingSpeak.setField(7, digitalRead(GREEN_PIN));  // Living (Verde)
  int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);
  if (x == 200) {
    Serial.println("Datos enviados correctamente a ThingSpeak");
  } else {
    Serial.println("Error al enviar los datos: " + String(x));
  }
  delay(20000); // cada 20 segundos
}
