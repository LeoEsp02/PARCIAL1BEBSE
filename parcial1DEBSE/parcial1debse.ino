
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define pin2 32  // Pin del DHT22
#define buzzerPin 13  // Pin para el buzzer (puedes cambiarlo si lo conectas a otro pin)
// Definir los umbrales para la alarma
#define TEMP_THRESHOLD 27.0  // Umbral de temperatura en grados Celsius
#define HUM_THRESHOLD 60.0   // Umbral de humedad en porcentaje

DHT dht2(pin2, DHT22);  // Crear objeto para el sensor DHT22
Adafruit_BMP280 bme;    // Crear objeto para el sensor BMP280

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int number1 = 0;
int number2 = 0;
int number3 = 0;
int number4 = random(0,100);
String myStatus = "";

void setup() {
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println("Test de sensores:");
  dht2.begin();
   Wire.begin(21, 22);  // SDA en GPIO21 y SCL en GPIO22
  if (!bme.begin(0x76)) {  // Probar con la dirección 0x76
    Serial.println(F("Error al inicializar el sensor BMP280"));
    while (1);  // Detener el programa si no se puede inicializar el sensor
  }
  Serial.println(F("Sensor BMP280 inicializado correctamente"));

  // Inicializar el buzzer como salida
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);  // Asegurarse de que el buzzer esté apagado al inicio
}

void loop() {
   leerdht2();

  // Leer y mostrar datos del sensor BMP280
  leerBMP280();

  delay(1000);

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  // set the fields with the values
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);
  ThingSpeak.setField(3, number3);

  // figure out the status message
  if(number1 > number2){
    myStatus = String("field1 is greater than field2"); 
  }
  else if(number1 < number2){
    myStatus = String("field1 is less than field2");
  }
  else{
    myStatus = String("field1 equals field2");
  }
  
  // set the status
  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(2000); // Wait 20 seconds to update the channel again
}

// Función que lee los valores del sensor DHT22 y los imprime en el Serial Monitor
void leerdht2() {
  float t2 = dht2.readTemperature();
  float h2 = dht2.readHumidity();

  // Verifica si la lectura es válida
  while (isnan(t2) || isnan(h2)) {
    Serial.println("Lectura fallida en el sensor DHT22, repitiendo lectura...");
    delay(2000);
    t2 = dht2.readTemperature();
    h2 = dht2.readHumidity();
  }
  number1=dht2.readTemperature();
  number2=dht2.readHumidity();
  // Imprime los valores de temperatura y humedad en el monitor serie
  Serial.print("Temperatura DHT22: ");
  Serial.print(t2);
  Serial.println(" ºC.");

  Serial.print("Humedad DHT22: ");
  Serial.print(h2);
  Serial.println(" %.");

  Serial.println("-----------------------");

  // Verificar si la temperatura o la humedad superan los umbrales
  if (t2 > TEMP_THRESHOLD || h2 > HUM_THRESHOLD) {
    activarAlarma();  // Activar alarma si se superan los umbrales
  } else {
    detenerAlarma();  // Detener alarma si los valores están dentro del rango
  }
}

// Función que lee los valores del sensor BMP280 y los imprime en el Serial Monitor
void leerBMP280() {
  // Leer y mostrar la temperatura, presión y altitud
  Serial.print(F("Temperatura BMP280: "));
  Serial.print(bme.readTemperature());
  Serial.println(F(" °C"));

  Serial.print(F("Presión BMP280: "));
  Serial.print(bme.readPressure() / 100.0F); // Convertir a hPa
  number3=bme.readPressure() / 100.0F;
  Serial.println(F(" hPa"));

  Serial.print(F("Altitud BMP280: "));
  Serial.print(bme.readAltitude(1013.25));  // Ajustar según la presión a nivel del mar
  Serial.println(F(" metros"));

  Serial.println("-----------------------");
}

// Función para activar la alarma (buzzer)
void activarAlarma() {
  Serial.println("¡ALERTA! Temperatura o Humedad fuera de rango.");
  digitalWrite(buzzerPin, HIGH);  // Encender el buzzer
}

// Función para detener la alarma (buzzer)
void detenerAlarma() {
  digitalWrite(buzzerPin, LOW);  // Apagar el buzzer
}