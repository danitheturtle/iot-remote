#include <stdint.h>
#include "TouchScreen.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "WiFi101.h"
#include "Adafruit_ILI9341.h"

// Touch screen coordinates (different for different screens)
#define TS_MINX 333
#define TS_MINY 220
#define TS_MAXX 820
#define TS_MAXY 870

// 6 buttons total
#define RED_BUTTON 0 //top-left - ILI9341_RED
#define YELLOW_BUTTON 1 //top-right - ILI9341_YELLOW
#define CYAN_BUTTON 2 //middle-left - ILI9341_CYAN
#define PURPLE_BUTTON 3 //middle-right - ILI9341_PURPLE
#define GREEN_BUTTON 4 //bottom-left - ILI9341_GREEN
#define BLUE_BUTTON 5 //bottom-right - ILI9341_BLUE

//Pin Defines
// The display also uses pins 4 and 5 for D/C and CS
#define TFT_DC 4
#define TFT_CS 5
//Touch screen com pins
#define YP A2  
#define XM A3  
#define YM 6   
#define XP 4
//Display toggle button
#define BUTTON_IN 7
//Touch screen power toggle
#define SCREEN_POW_OUT 3

//Global variables
#define TOUCHRADIUS 3
int currentcolor;
int isDisplayOff = 0;
int lastButtonState = 0;
int buttonState = 0;

//Wifi variables
char ssid[] = "danswifi29";
char pass[] = "5137390508";
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiClient client(80);
char server[] = "maker.ifttt.com";
unsigned long lastConnectionTime = 0; //Last time we connected to server in ms
const unsigned long postingInterval = 10L * 1000L; //Delay between updates in ms

char redEndpoint[] = "/trigger/red_iot_button/with/key/dAzBOm-Bxih8Jz9IPjKx1r";
char yellowEndpoint[] = "/trigger/yellow_iot_button/with/key/dAzBOm-Bxih8Jz9IPjKx1r";
char cyanEndpoint[] = "/trigger/cyan_iot_button/with/key/dAzBOm-Bxih8Jz9IPjKx1r";
char purpleEndpoint[] = "/trigger/purple_iot_button/with/key/dAzBOm-Bxih8Jz9IPjKx1r";
char greenEndpoint[] = "/trigger/green_iot_button/with/key/dAzBOm-Bxih8Jz9IPjKx1r";
char blueEndpoint[] = "/trigger/blue_iot_button/with/key/dAzBOm-Bxih8Jz9IPjKx1r";

bool requestActive = false;

//Set up touch screen and TFT display
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup(void) {
  //Begin serial connection and device polling
  Serial.begin(9600);
  //Start display
  tft.begin();

  //Set up pins
  pinMode(BUTTON_IN, INPUT);
  pinMode(SCREEN_POW_OUT, OUTPUT);
  //analogWrite(SCREEN_POW_OUT, 0);

  //Black background
  clearScreen();
 
  //select the current color 'red'
  currentcolor = ILI9341_RED;
  
  //Make sure wifi is ready to be set up
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
  } else {
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network:
      status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(5000);
    }
    //Print connection details
    printWiFiData();
    drawButtons();
  }
}


void loop() {
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
//  if (!client.connected() && requestActive == true) {
//    analogWrite(SCREEN_POW_OUT, 255);
//    delay(200);
//    analogWrite(SCREEN_POW_OUT, 0);
//    delay(150);
//    analogWrite(SCREEN_POW_OUT, 255);
//    delay(200);
//    analogWrite(SCREEN_POW_OUT, 0);
//    requestActive = false;
//  }

  //Hardware button edge detection
  buttonState = digitalRead(BUTTON_IN);
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      if (isDisplayOff == 1) {
        drawButtons();
        isDisplayOff = 0;
      } else {
        clearScreen();
        isDisplayOff = 1;
      }
    }
    delay(50);
  }
  lastButtonState = buttonState;

  if (isDisplayOff == 0) {
    // Retrieve a point  
    TSPoint p = ts.getPoint();
  
    //Scale resistor touch screen data into coordinate space
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    
    //If this is a valid touch location, 
    if (p.z > 100 && p.z < 1000 && !requestActive) {
      //Draw something
      int touchedButton = getCurrentButton(p.x, p.y);
      screenButtonPushed(touchedButton);
    }
  } else {
    delay(50);
  }
}

void screenButtonPushed(int _index) {
  requestActive = true;
//  analogWrite(SCREEN_POW_OUT, 200);
//  delay(100);
//  analogWrite(SCREEN_POW_OUT, 0);
  if (client.connect(server, 80)) {
    client.print("GET ");
    switch (_index) {
      case (RED_BUTTON):
        client.print(redEndpoint);
        break;
      case (YELLOW_BUTTON):
        client.print(yellowEndpoint);
        break;
      case (CYAN_BUTTON):
        client.print(cyanEndpoint);
        break;
      case (PURPLE_BUTTON):
        client.print(purpleEndpoint);
        break;
      case (GREEN_BUTTON):
        client.print(greenEndpoint);
        break;
      case (BLUE_BUTTON):
        client.print(blueEndpoint);
        break;
      default:
        client.stop();
        return;
        break;
    }
    client.println(" HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
  } else {
    requestActive = false;
  }
}

void printWiFiData() {
  //Network name
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: "); Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI(); Serial.print("signal strength (RSSI):"); Serial.println(rssi);
  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:"); Serial.println(encryption, HEX); Serial.println();
}

void clearScreen() {
  tft.fillScreen(ILI9341_BLACK);
}

void drawButtons() {
  int thisWidth = tft.width();
  int thisHeight = tft.height();
  //Draw red
  tft.fillRect(0, 0, thisWidth/2, thisHeight/3, ILI9341_RED);
  //Draw yellow
  tft.fillRect(thisWidth/2, 0, thisWidth/2, thisHeight/3, ILI9341_YELLOW);
  //Draw cyan
  tft.fillRect(0, thisHeight/3, thisWidth/2, thisHeight/3, ILI9341_CYAN);
  //Draw purple
  tft.fillRect(thisWidth/2, thisHeight/3, thisWidth/2, thisHeight/3, ILI9341_PURPLE);
  //Draw green
  tft.fillRect(0, 2*thisHeight/3, thisWidth/2, thisHeight/3, ILI9341_GREEN);
  //Draw blue
  tft.fillRect(thisWidth/2, 2*thisHeight/3, thisWidth/2, thisHeight/3, ILI9341_BLUE);
}

int getCurrentButton(int x, int y) {
  int thisWidth = tft.width();
  int thisHeight = tft.height();
  //Check red
  if (x < thisWidth/2 && y < thisHeight/3) {
    return RED_BUTTON;
  }
  //Check yellow
  if (x >= thisWidth/2 && y < thisHeight/3) {
    return YELLOW_BUTTON;
  }
  //Check cyan
  if (x < thisWidth/2 && y >= thisHeight/3 && y <= 2*thisHeight/3) {
    return CYAN_BUTTON;
  }
  //Check purple
  if (x >= thisWidth/2 && y >= thisHeight/3 && y <= 2*thisHeight/3) {
    return PURPLE_BUTTON;
  }
  //Check green
  if (x < thisWidth/2 && y > 2*thisHeight/3) {
    return GREEN_BUTTON;
  }
  //Check blue
  if (x >= thisWidth/2 && y > 2*thisHeight/3) {
    return BLUE_BUTTON;
  }
}
