#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AiEsp32RotaryEncoder.h"
#include <Keypad.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Button2.h"


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define ROTARY_ENCODER_A_PIN 33
#define ROTARY_ENCODER_B_PIN 32
#define ROTARY_ENCODER_BUTTON_PIN 26
#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */
//#define ROTARY_ENCODER_STEPS 1
//#define ROTARY_ENCODER_STEPS 2
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

#define BUTTON_PIN 25
Button2 button;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
byte keypad_page = 0; 
byte display_page = 0;
int media_val = 0;
int led_val = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte ROWS = 3; //four rows
const byte COLS = 5; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {1,2,3,4,5},
  {6,7,8,9,10},
  {11,12,13,14,15},
};
byte rowPins[ROWS] = {5, 18, 19}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 13, 15, 23, 4}; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}


//-----------------------------------------------------------------------------------------------------------------------------------

void rotary_onButtonClick()
{
	static unsigned long lastTimePressed = 0;
	//ignore multiple press in that time milliseconds
	if (millis() - lastTimePressed < 500)
	{
		return;
	}
	lastTimePressed = millis();
  Serial.print("button pressed ");
  
  JsonDocument doc;
  doc["type"] = "enc";
  doc["action"] = "clicked";
  doc["screen"] = display_page;
  String output;
  serializeJson(doc, output);
  doc.clear();
  ws.textAll(output);
}

void released(Button2& btn) {
    display_page++;
    display_page_switch();
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
 
  } else if(type == WS_EVT_DATA){
 
    Serial.println("Data received: ");
    String json_sync = (char*)data;
    Serial.println(json_sync);


    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json_sync);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    media_val = doc["sync"]["media"];
    
    led_val = doc["sync"]["led"];
    enc_sync();


    doc.clear();
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------



void enc_sync()
{
  switch (display_page) {
    case 0:
          break;
    case 1:
          rotaryEncoder.setEncoderValue(media_val);
          break;
    case 2:
          rotaryEncoder.setEncoderValue(led_val);
          break;
    case 3:
          break;
  }
}

void enc_change()
{
  switch (display_page) {
    case 0:
          break;
    case 1:
          media_val = rotaryEncoder.readEncoder();
          break;
    case 2:
          media_val = rotaryEncoder.readEncoder();
          break;
    case 3:
          break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------------

void draw_startup_display()
{
  //ip 
  display.clearDisplay();
  display.setRotation(0);
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0, 0);
  display.println(WiFi.localIP());
  display.println(F("Kliknij wyświetlacz aby kontynuować"));
  display.display(); 
}
void draw_media_display()
{
  display.clearDisplay();
  display.setRotation(3);
	display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0, 0);
  display.println(F("MEDIA"));
  display.drawRect(0,26,32,102,SSD1306_WHITE);
  display.display();
}
void draw_led_display()
{
  display.clearDisplay();
  display.setRotation(3);
	display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0, 0);
  display.println(F("LED"));
  display.drawRect(0,26,32,102,SSD1306_WHITE);
  display.display();
}



void media_display_update()
{
  //display.clearDisplay();
  display.fillRect(1,27,30,100,SSD1306_BLACK); //clear bar
  display.fillRect(1,27+(100-media_val),30,media_val,SSD1306_WHITE);//update from value
  display.display();
}
void led_display_update()
{
  //display.clearDisplay();
  display.fillRect(1,27,30,100,SSD1306_BLACK); //clear bar
  display.fillRect(1,27+(100-led_val),30,led_val,SSD1306_WHITE);//update from value
  display.display();
}



void display_page_switch()
{
  switch (display_page) {
    case 0:
      draw_startup_display();
      break;
    case 1:
      draw_media_display();
      rotaryEncoder.setEncoderValue(media_val);
      break;
    case 2:
      draw_led_display();
      rotaryEncoder.setEncoderValue(led_val);
      break;
    case 3:
      display_page = 0;
      display_page_switch();
      break;

  }
}

//--------------------------------------------------------------------------------------------------------------------------------

void rotary_loop()
{
	//dont print anything unless value changed
	if (rotaryEncoder.encoderChanged())
	{
		enc_change();
    JsonDocument doc;
    doc["type"] = "enc";
    doc["action"] = "rotation";
    doc["value"] = media_val;
    doc["screen"] = display_page;
    String output;
    serializeJson(doc, output);
    doc.clear();
    ws.textAll(output);

	}
	if (rotaryEncoder.isEncoderButtonClicked())
	{
		rotary_onButtonClick();
	}
}

void display_loop()
{
  switch (display_page) {
    case 0:
          break;
    case 1:
          media_display_update();
          break;
    case 2:
          led_display_update();
          break;
  }
}

//---------------------------------------------------------------------------------------------------------------------------------

void setup()
{
	Serial.begin(115200);
  WiFiManager wm;

  bool res;
  res = wm.autoConnect("keypadAP","12345678"); // password protected ap
  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  
  server.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  display.clearDisplay();
  display.display();

	rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
	bool circleValues = false;
	rotaryEncoder.setBoundaries(0, 100, circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
	rotaryEncoder.setAcceleration(100); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

  button.begin(BUTTON_PIN);
  button.setReleasedHandler(released);
  display_page_switch();
}

void loop()
{
	//in loop call your custom function which will process rotary encoder values
	rotary_loop();
  button.loop();
  display_loop();
  ws.cleanupClients();
  
  char customKey = customKeypad.getKey();
  if (customKey){
    Serial.println(customKey);
    JsonDocument doc;
    doc["type"] = "keypad";
    doc["button"] = (unsigned char)customKey;
    doc["page"] = keypad_page;
    String output;
    serializeJson(doc, output);
    doc.clear();
    ws.textAll(output);
  }
  


}

