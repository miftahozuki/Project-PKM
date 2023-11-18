#include <WiFi.h>
#include <ArduinoOTA.h>
#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

/* Wifi Crdentials */
String sta_ssid = "KRC-BBOGA";     // set Wifi network you want to connect to
String sta_password = "1290zxc1290";   // set password for Wifi network

/* Defining motor and servo pins */
extern int DRV_A = 17;
extern int DRV_B = 5;
extern int DIR_A = 18;
extern int DIR_B = 19;

extern int lampu = 15;
extern int ledIndicator = 27;
extern int buzzerPin = 26;  // set digital pin GPIO26 as LED pin (use Active Buzzer)
extern int servoPin = 2;  // set digital pin GPIO2 as servo pin (use SG90)
extern int ledVal = 20;  // unused
extern int ledPin = 25;  // unused_pin

int voltage_offset = 65;// set the correction offset value
int bat_percentage;

const int lebar=128;
const int tinggi=64;
const int reset=3;

Adafruit_SSD1306 oled(lebar,tinggi,&Wire,reset);

unsigned long previousMillis = 0;

void startCameraServer();

void initServo() {
  ledcSetup(8, 50, 16); /*50 hz PWM, 16-bit resolution and range from 3250 to 6500 */
  ledcAttachPin(servoPin, 8);
}
//
void initLed() {
  ledcSetup(7, 5000, 8); /* 5000 hz PWM, 8-bit resolution and range from 0 to 255 */
  ledcAttachPin(ledPin, 7);
  ledcWrite(8, 3250);
}

void setup() {
  Serial.begin(115200);         // set up seriamonitor at 115200 bps
  Serial.setDebugOutput(true);
  Serial.println();

  //OLED
  oled.begin(SSD1306_SWITCHCAPVCC,0x3C);
  oled.clearDisplay();  
  
  Serial.println("*ESP32 Kit Remote Control*");
  Serial.println("--------------------------------------------------------");

  // Set all the motor control pin to Output
  pinMode(DRV_A, OUTPUT);
  pinMode(DRV_B, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  
  pinMode(ledPin, OUTPUT); // set the LED pin as an Output
  pinMode(lampu, OUTPUT);    
  pinMode(ledIndicator, OUTPUT);  
  pinMode(buzzerPin, OUTPUT); // set the buzzer pin as an Output
  pinMode(servoPin, OUTPUT); // set the servo pin as an Output

  // Initial state - turn off motors, LED & buzzer
  digitalWrite(DRV_A, LOW);
  digitalWrite(DRV_B, LOW);
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(servoPin, LOW);
//  ledcWrite(8, 3250);

  /* Initializing Servo and LED */
  initServo();
  initLed();

  // Set NodeMCU Wifi hostname based on chip mac address
  char chip_id[15];
  snprintf(chip_id, 15, "%04X", (uint16_t)(ESP.getEfuseMac()>>32));
  String hostname = "KRC-BBOGA";

  Serial.println();
  Serial.println("Hostname: "+hostname);

  // first, set NodeMCU as STA mode to connect with a Wifi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.println("");
  Serial.print("Connecting to: ");
  Serial.println(sta_ssid);
  Serial.print("Password: ");
  Serial.println(sta_password);

  // try to connect with Wifi network about 10 seconds
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000) {
    delay(500);
    Serial.print(".");
    currentMillis = millis();
  }

  // if failed to connect with Wifi network set NodeMCU as AP mode
//  IPAddress myIP;
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("*WiFi-STA-Mode*");
    Serial.print("IP: ");
//    myIP=WiFi.localIP();  
//    Serial.println(myIP);
    delay(2000);
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str(), sta_password.c_str());
//    myIP = WiFi.softAPIP();
//    digitalWrite(ledIndicator, HIGH); 
    Serial.println("");
    Serial.println("WiFi failed connected to " + sta_ssid);
    Serial.println("");
    Serial.println("*WiFi-AP-Mode*"); 
//    Serial.print("AP IP address: ");
//    Serial.println(myIP);
    delay(2000);
  }

  // Start camera server to get realtime view
  startCameraServer();
//  Serial.print("Camera Ready! Use 'http://");
//  Serial.print(myIP);
//  Serial.println("' to connect ");

  ArduinoOTA.begin();   // enable to receive update/upload firmware via Wifi OTA
}

void loop() {
  //Voltage
  int volt = analogRead(34);// read the input - GPIO_NUM_34
  double voltage = map(volt,0, 4096, 0, 1650) + voltage_offset;

  voltage /= 100; // divide by 100 to get the decimal values
  bat_percentage = mapfloat(voltage, 5.1, 8.1, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
 
  if (bat_percentage >= 100)
  {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0)
  {
    bat_percentage = 1;
  }
  
  Serial.print("Voltage: ");
  Serial.print(voltage + 0.08);
  Serial.println("V");

  delay(500);
  
  oled.begin(SSD1306_SWITCHCAPVCC,0x3C);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(12,15); 
  
  //Indicator
  IPAddress myIP;
  if (WiFi.status() == WL_CONNECTED) {
    oled.setCursor(12,25);
    oled.println("Mode : STATION");
    digitalWrite(ledIndicator, HIGH);
    delay(150);
    digitalWrite(ledIndicator, LOW);
    myIP=WiFi.localIP();
    oled.setCursor(12,35);
    oled.print("IP : ");  
    oled.print(myIP);    
    oled.display();      
    delay(80);
    } else {
    digitalWrite(ledIndicator, HIGH);
    oled.setCursor(12,25);
    oled.println("Mode : Access Point");
    myIP = WiFi.softAPIP();
    oled.setCursor(12,35);
    oled.print("IP : ");
    oled.print(myIP);      
    oled.display();      
      }  
      
  // OLED
//  oled.setTextSize(1);
//  oled.setTextColor(WHITE);
  oled.setCursor(12,15);
  oled.println("Name : " +sta_ssid);
//  oled.setCursor(12,25);
//  oled.println("Mode : " +xyz);
//  oled.setCursor(12,35);
//  oled.println("IP : Unknown");  
//  oled.display();
  oled.setCursor(12,45);
  oled.print("Battery : ");      
  oled.print(bat_percentage);
  oled.print("%");     
  oled.display();     
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
