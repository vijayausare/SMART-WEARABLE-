/*********SERIAL COMMUNICATION****************************************************/
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
///Initialise Arduino to NodeMCU (0=Rx & 1=Tx)
//SoftwareSerial nodemcu(0, 1);

String message = "";
bool messageReady = false;


/*********PULSE SENSOR****************************************************/
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library
const int PulseWire = A0;       // 'S' Signal pin connected to A0
const int LED13 = 13;          // The on-board Arduino LED
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore
PulseSensorPlayground pulseSensor;  // Creates an object

const int soilMoisture = 2;

/**** MQ4****/
const int MQ4 = 3;

/*****DHT11****/
#include "DHT.h"
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

/******LCD DISPLAY***************/
#include <LiquidCrystal.h>

const int rs = 5;
const int en = 6;
const int D4 = 7;
const int D5 = 8;
const int D6 = 9;
const int D7 = 10;
LiquidCrystal lcd(rs, en, D4, D5, D6, D7);
//////////////////////////////////////////////////////////////////////
void setup() {

  /******LCD DISPLAY***************/
  lcd.begin(16, 2);  // set up the LCD's number of columns and rows:
  lcd.setCursor(1, 0);
  lcd.print(" TEAM ADMIRALS");  // Print a text to the LCD.
  delay(4000);

  Serial.begin(9600);
  //  nodemcu.begin(9600);
  delay(1000);

  Serial.println("Program started");

  /**** MQ4****/
  pinMode(MQ4, INPUT);

  /*****DHT11*********************************************************/
  Serial.println(F("DHTxx test!"));
  dht.begin();

  /*********PULSE SENSOR****************************************************/
  // Configure the PulseSensor object, by assigning our variables to it
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED13);       // Blink on-board LED with heartbeat
  pulseSensor.setThreshold(Threshold);

  // Double-check the "pulseSensor" object was created and began seeing a signal
  if (pulseSensor.begin()) {
    Serial.println("PulseSensor object created!");
  }

}
//////////////////////////////////////////////////////////////////////
void loop() {
  delay(2000);
  //    StaticJsonBuffer<1000> jsonBuffer;
  //    JsonObject& data = jsonBuffer.createObject();

  lcd.clear();
  /**** MQ4****************************************************************/
  const int isGas = digitalRead(MQ4);
  //  data["smoke"] = isGas;
  if (!isGas) {
    Serial.println("SMOKE DETECTED!");
    lcd.setCursor(0, 0);
    lcd.print("SMOKE DETECTED!");
  }
  else {
    Serial.println("SMOKE NOT DETECED !");
    lcd.setCursor(0, 0);
    lcd.print("SMOKE NOT DETECTED!");
  }
  delay(1800);

  /*****DHT11************************************************************/

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  delay(250);
  float t = dht.readTemperature();// Read temperature as Celsius (the default)
  //  data["humidity"] = h;
  //  data["temp"] = t;

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TEMPERATURE");
  lcd.setCursor(0, 1);
  lcd.print("VALUE : ");
  lcd.setCursor(10, 1);
  lcd.print(t);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HUMIDITY");
  lcd.setCursor(0, 1);
  lcd.print("VALUE : ");
  lcd.setCursor(10, 1);
  lcd.print(h);
  delay(2000);
  /*********PULSE SENSOR****************************************************/

  int myBPM = pulseSensor.getBeatsPerMinute();      // Calculates BPM
  //  data["bpm"] = myBPM;


  if (pulseSensor.sawStartOfBeat()) {               // Constantly test to see if a beat happened
    Serial.println("HeartBeat Happened ! "); // If true, print a message
    Serial.print("BPM: ");
    Serial.println(myBPM);                        // Print the BPM value
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BPM");
  lcd.setCursor(0, 1);
  lcd.print("VALUE : ");
  lcd.setCursor(10, 1);
  lcd.print(myBPM);


  //Send data to NodeMCU
  //  data.printTo(nodemcu);
  //  jsonBuffer.clear();


  // Monitor serial communication
  while (Serial.available()) {
    message = Serial.readString();
    messageReady = true;
  }
  // Only process message if there's one
  if (messageReady) {
    // The only messages we'll parse will be formatted in JSON
    DynamicJsonDocument doc(1024); // ArduinoJson version 6+
    // Attempt to deserialize the message
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }
    if (doc["type"] == "request") {
      doc["type"] = "response";
      // Get data from analog sensors
      doc["gas"] = isGas;
      doc["temp"] = t;
      doc["hum"] = h;
       doc["bpm"] = myBPM;

      serializeJson(doc, Serial);
    }
    messageReady = false;
  }

  delay(2000);


}
