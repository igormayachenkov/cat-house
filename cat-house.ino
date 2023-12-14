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
OneWire wireA(13);
OneWire wireB(27);

DallasTemperature sensA(&wireA);
DallasTemperature sensB(&wireB);

// Heater control
float tempTarget;
float hysteresis = 0.5;
float tempOn;  // low  bound
float tempOff; // high bound
void setTarget(float newTarget){
  if(newTarget<0 || newTarget>15) return;
  tempTarget = newTarget;
  tempOn  = tempTarget - hysteresis; // low  bound
  tempOff = tempTarget + hysteresis; // high bound
}

// Wi-Fi and Timing
const int WIFI_LOOP = 3; // MAX number of the main loops
const int MAIN_LOOP = 10000; // ms
int wifiLoops = 0; // 
const char* ssid     = "MGTS_GPON_1178";
const char* password = "K1V03ZUV";

// Wi-Fi Client
WiFiClient client;
IPAddress BACKEND_IP(188,120,249,76);
//IPAddress BACKEND_IP(192,168,1,68);
const int BACKEND_PORT = 8888; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Target T
  setTarget(5.0);

  // Init output pins
  pinMode(HEATER_PIN,OUTPUT);
  setHeating(true);

  // Start the DS18B20 sensor
  sensA.begin();
  sensB.begin();

}

void loop() {

  // Measure Temperature
  sensA.requestTemperatures(); 
  sensB.requestTemperatures(); 
  float tempA = sensA.getTempCByIndex(0);
  float tempB = sensB.getTempCByIndex(0);

  // Control HEATER
  if(tempA<tempOn && !heating){
    setHeating(true);
  }
  if(tempA>tempOff && heating){
    setHeating(false);
  }

  // Console output
  Serial.print("Temp C: ");
  Serial.print(tempA);
  Serial.print(" ");
  Serial.print(tempB);
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

      //----- SEND REQUEST -----
      //client.stop();
      if (client.connect(BACKEND_IP, BACKEND_PORT)) {
        client.printf(
          "tempA:%.2f tempTarget:%.2f heating:%d tempB:%.2f",
          tempA,tempTarget,heating,tempB);
        delay(5000);
        // Read        
        while (client.available()) {
          // char c = client.read();
          // Serial.write(c);
          String str = client.readString();
          Serial.print(str);
          int newTarget = str.toInt();
          setTarget(newTarget);
        } 
        // Close        
        client.flush();
        client.stop();
        Serial.println("\ndata sent");    
      }else{
        Serial.println("client connection failed");    
      }
      //--------------------------------------

      WiFi.disconnect();
    }

  }



  delay(MAIN_LOOP);

}
