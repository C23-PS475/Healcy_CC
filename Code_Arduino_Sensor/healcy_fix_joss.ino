#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_MLX90614.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#define FIREBASE_HOST "healcy-388714-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "nEgH0aWBYb4cLnH8EXPssHJ9WRO9e96eGIrtrG5w"

#define WIFI_SSID "Affandy"
#define WIFI_PASSWORD "ariefsatrio17"

FirebaseData firebaseData;

#define MAX30105_ADDRESS 0x57
#define MLX90614_ADDRESS 0x5A


MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 
uint32_t tsLastReport = 0;
unsigned long previousMillis = 0; // Time since the last data update
const unsigned long interval = 2000; // Interval between data updates (in milliseconds)

float beatsPerMinute;
int beatAvg;
float TargetC;
int analogPin = 32;  // Use GPIO32 for analog input on ESP32
float ureterContraction;
float baselineValue;
float fetalMovement;

void sendToFirebase() {
  Firebase.setInt(firebaseData, "Jantung", beatAvg);
}
void sendToFirebase2() {
  Firebase.setInt(firebaseData, "/Kontraksi/ureter_contraction", ureterContraction);
  Firebase.setInt(firebaseData, "/Kontraksi/baseline_value", baselineValue);
  Firebase.setInt(firebaseData, "/Kontraksi/fetal_movement", fetalMovement);
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");

  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected");
  Serial.println(WiFi.localIP());
  

  
   // Initialize sensor MAX
   if (!particleSensor.begin(Wire, MAX30105_ADDRESS)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  
  delay(100); // Tambahkan delay sebelum memanggil fungsi berikutnya
  
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  // Initialize sensor MLX
  Wire.beginTransmission(MLX90614_ADDRESS);
  Wire.endTransmission();

  pinMode(analogPin, INPUT);
  mlx.begin(); // Remove the address parameter
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // put your setup code here, to run once:

}

void getMAXValue(void) 
{
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true && millis() - lastBeat > 1000)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  Serial.println();
  delay(10);
  
 unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    sendToFirebase();
    previousMillis = currentMillis;
  }
  if (irValue < 50000){
  beatsPerMinute = 0;
  beatAvg = 0;
  Serial.print(" No finger?");
  Serial.println();
}
}

void getMLXValue(void) 
{
  if(millis() - tsLastReport > 5000){
    TargetC = mlx.readObjectTempC();
    Serial.print("Suhu : ");
    Serial.println(TargetC);
    tsLastReport = millis();
    Firebase.setInt(firebaseData, "Suhu", TargetC);
  }
}

void getPiezoValue(void) 
{
  ureterContraction = analogRead(analogPin) * 0.0018 / 4095.0;  // Scale: 0.0001 - 0.0019
  baselineValue = analogRead(analogPin) * 110.0 / 4095.0 + 90.0;  // Scale: 90-200
  fetalMovement = analogRead(analogPin) * 9.0 / 4095.0;  // Scale: 0-9
  Serial.print("Ureter Contraction = ");
  Serial.print(ureterContraction, 4);
  Serial.println();
  Serial.print("Baseline Value = ");
  Serial.print(baselineValue, 2);
  Serial.println();
  Serial.print("Fetal Movement = ");
  Serial.print(fetalMovement, 2);
  Serial.println();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    sendToFirebase2();
    previousMillis = currentMillis;
  }
  
}
void loop() {
 getMAXValue();
 getMLXValue();
 getPiezoValue();
  // put your main code here, to run repeatedly:

}
