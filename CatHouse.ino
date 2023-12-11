//#include <LiquidCrystal.h> //https://www.arduino.cc/reference/en/libraries/liquidcrystal/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>


// Heater
const int HEATER_PIN = 22;
bool heating = false;
void setHeating(bool val){
  heating = val;
  digitalWrite(HEATER_PIN, heating?LOW:HIGH); // low - heater is on
}

// Temperature measurement
OneWire wireSt(13);
OneWire wireHm(27);

DallasTemperature sensorsSt(&wireSt);
DallasTemperature sensorsHm(&wireHm);

// Heater control
float tempTarget = 20.0;
float hysteresis = 0.5;
float tempOn  = tempTarget - hysteresis; // low  bound
float tempOff = tempTarget + hysteresis; // high bound

// Wi-Fi and Timing
const int WIFI_LOOP = 3; // MAX number of the main loops
const int MAIN_LOOP = 10000; // ms
int wifiLoops = 0; // 
const char* ssid     = "MGTS_GPON_1178";
const char* password = "K1V03ZUV";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Init output pins
  pinMode(HEATER_PIN,OUTPUT);
  setHeating(true);

  // Start the DS18B20 sensor
  sensorsSt.begin();
  sensorsHm.begin();

}

void loop() {

  // Measure Temperature
  sensorsSt.requestTemperatures(); 
  sensorsHm.requestTemperatures(); 
  float tempSt = sensorsSt.getTempCByIndex(0);
  float tempHm = sensorsHm.getTempCByIndex(0);

  // Control HEATER
  if(tempSt<tempOn && !heating){
    setHeating(true);
  }
  if(tempSt>tempOff && heating){
    setHeating(false);
  }

  // Console output
  Serial.print("Temp C: ");
  Serial.print(tempSt);
  Serial.print(" ");
  Serial.print(tempHm);
  Serial.print("   target: ");
  Serial.print(tempTarget);
  Serial.print(" => heating: ");
  Serial.println(heating);

  // Wi-Fi Output
  wifiLoops++;
  if(wifiLoops>=WIFI_LOOP){
    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
    }else{
      Serial.printf("WiFi connected!\n");
      wifiLoops=0;
      WiFi.disconnect();
    }

  }



  delay(MAIN_LOOP);

}
