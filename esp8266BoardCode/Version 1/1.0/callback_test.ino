#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>

//#define mqtt_server "104.236.210.175" //fill this
#define mqtt_server "159.203.102.9" //fill this
#define mqtt_user "" // no fill necessary
#define mqtt_password "" // no fill necessary
//#define topic1 "temp/temp1"
//#define topic1 "esp_temp_test3"
Ticker sleepTicker;


// THIS IS WHERE THINGS NEED TO BE DEFINED
//***************************************************************
#define topic1 "esp_temp_test2"
#define mqtt_client_name "201" //dont change
//set the temp wire input here
#define MAX_LOOP_TIME_MS     15000
  


/*
 * 
 * FEATURES FOR THIS SCRIPT
 * 
 * -added a wifi timeout so users have 90 seconds to connect before it resets
 *    -rational for this was that if a users wifi goes down, theres currently no way to have the device not connect as an AP if it notices the wifi is down
 *    -TODO fix this 
 * -timeout in loop once connected to wifi
 * - temp sensor is usally sending out 185 as its first value, so changed that so it loops and checks again for new values
 * - if the temp sensor is not connected, the device will send -197 degrees. Changed this to 1000 so we can flag it when it happens
 * 
 * 
 * NOTE you MUST run this script on the chip with wifi.reset() uncommented to clear the EEPROM, then rerun it with that commented out
 * note you must also change the wifi at the bottom to edit the deviceID into it
 * TODO fix comment above
 * 
 * 
 * 
 * LIFECYCLE OF APPLICATION
 * 
 * STARTUP - LED GOES ORANGE
 * IF CONNECTS TO WIFI, BLINK GREEN
 * if doesnt connect, goes red
 * if loses connection to mqtt, blinks red
 * if timeout, goes red
 */

 







///the following pins work on the ESP8266 ESP-12 to read the temperature
//  14
// pin 2 works but wouldn't suggest it due to it being on the same side as ground, not vcc which needs a pullup resistor 
// 13 
//PLEASE NOTE THAT THE PINS THAT DON'T WORK BELOW MAY BE DUE TO A WIRING FLAW ... but i doubt it :)
// 12 DOES NOT WORK

 #define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient client(espClient);
int deviceID = 201;
String WiFiDevID = "test";



int redPin = 5;
int greenPin = 4;
int bluePin = 14;

int redArr[]= {0,255,0};
int greenArr[] = {255,0,0};
int blueArr[]= {0,0,255};


int yellowArr[] = {255, 255, 0};
int orangeArr[] = {40, 255, 0};
//int orangeArr[] = {128, 255, 0};
int whiteArr[] = {255, 128, 0};
int blackArr[] = {0,0,0};

char current_temp[32];
//this is in seconds

int sleepTimeS = 30;
//sleeptimeS_AP is for when the device doesn't connect to the network
int sleepTimeS_AP = 60;
int wifi_timeout = 90;

unsigned long startTime;


//NOTE TO CHECK WIFI.RESET SETTINGS DONT MESS THAT UP.
//ON FIRST PUSH MAKE SURE TO RUN RESET SETTINGS, THEN PUSH CODE AGAIN AND COMMENT THAT OUT.
//********************************************************


void setColor(int color[])
{
    int red;
    int green;
    int blue;
    
#ifdef COMMON_ANODE
    red = 255 - color[0];
    green = 255 - color[1];
    blue = 255 - color[2];

  #endif

    setColor1(color[0],color[1],color[2]);


}


void blink_lights(int light1[],int light2[], int counter, int time_delay){

   int i;
   for(i = 0; i < counter; i++){
    
    setColor(light1);
    delay(time_delay);
    setColor(light2);
    delay(time_delay);
     }
}


void setColor1(int red, int green, int blue)

  {
      #ifdef COMMON_ANODE
      red = 255 - red;
      green = 255 - green;
      blue = 255 - blue;
      #endif
  
      analogWrite(redPin, red);
      analogWrite(greenPin, green);
      analogWrite(bluePin, blue);
  }




void pubMQTT(String topic,String topic_val){
    Serial.print("Newest topic " + topic + " value:");
    Serial.println(String(topic_val).c_str());
    client.publish(topic.c_str(), String(topic_val).c_str(), true);
    blink_lights(greenArr,blackArr,2,75);
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(mqtt_client_name)) { //* See //NOTE below
            Serial.println("connected");
            blink_lights(greenArr,blackArr,2,75);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            
             blink_lights(blueArr,blackArr,8,75); 
                ESP.deepSleep(sleepTimeS * 10000000,WAKE_RF_DEFAULT); 

            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
  

void sleepyTime() {
  const int elapsed = millis() - startTime;
  Serial.println(millis());
  Serial.println(startTime);
  Serial.printf("Sleeping. Loop took %d ms\n", elapsed);
  if (elapsed >= MAX_LOOP_TIME_MS) {
  
    setColor(redArr);
    delay(2000);
   ESP.deepSleep(sleepTimeS * 10000000,WAKE_RF_DEFAULT); 
   }
  // It can take a while for the ESP to actually go to sleep.
  // When   it wakes up we start again in setup().
  delay(5000);
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // print the ssid that we should connect to to configure the ESP8266
  Serial.print("Created config portal AP ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
}




void setup() {
    Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  //SET THIS TO ORANGE IN PRODUCTION
  
  setColor(orangeArr);

  WiFiManager wifiManager;

    wifiManager.setAPCallback(configModeCallback);



  // you have 90 seconds to connect or youre booted
  //TODO until I figure out a way to read connection variables being set, the device will turn into an AP if the wifi is down or youre not connected.
  //setting to 90 seconds now
  wifiManager.setTimeout(wifi_timeout);
  //exit after config instead of connecting
  
  wifiManager.setBreakAfterConfig(true);
  
  //reset settings - for testing

  
  /////////////////////RESET STUFF HERE//////////////////////////////////////


  
  //wifiManager.resetSettings();



 ///////////////////////////////////////////////////


                  
  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  //if (!wifiManager.autoConnect("AutoConnectAP1234")) {
    if (!wifiManager.autoConnect("Device201")) {


    blink_lights(redArr,blackArr,8,75);
    Serial.println("failed to connect, we should reset as see if it connects");
    //setColor(redArr);
    delay(1000);
    ESP.reset();
      ESP.deepSleep(sleepTimeS_AP * 1000000,WAKE_RF_DEFAULT);
    delay(500);
  }
    wifiManager.autoConnect("Device201");
    //wifiManager.autoConnect("AutoConnectAP1234");
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  //blink_lights(green_pin,6,200);

//CONNECT TO WIFI GO GREEN
  
    blink_lights(greenArr,blackArr,3,50);
    delay(200);
   // blink_lights(yellowArr,orangeArr,6,50);
    //blink_lights(whiteArr,blackArr,6,50);
      
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);

}

void loop() {



      startTime = millis();
      sleepTicker.once_ms(16000, &sleepyTime);

      sensors.requestTemperatures(); // Send the command to get temperatures
      Serial.println("temp below");
      float temp_float = sensors.getTempFByIndex(0);
      Serial.println("first reading");
      Serial.println(temp_float);

        Serial.println("------------------");  

       // on the first reading it spits out 185 degrees if it measures something, not sure why but just make it rerequest a value

      while(temp_float > 184.5 && temp_float < 185.5){
        Serial.println("temp not set yet");
        sensors.requestTemperatures(); // Send the command to get temperatures
      temp_float = sensors.getTempFByIndex(0);
      Serial.println(temp_float);

      }
                  
      if (!client.connected()) {
        reconnect();
        }
  
    
    String temperature = dtostrf(temp_float, 4, 0, current_temp);
    temperature.trim();

    if (temperature == "-197"){
      temperature = "-1000";
    }
    
  
    String message_to_send1 = String("{\"devID\":") +  
    String(deviceID) + 
    ", \"current_temp\":" + temperature +
     + "}";  

     Serial.println(message_to_send1);
    pubMQTT(topic1, message_to_send1);
      


    //Serial.println("i will go to sleep now");
    delay(2000);
   ESP.deepSleep(sleepTimeS * 1000000,WAKE_RF_DEFAULT);
   delay(100);  
   Serial.println("this should not print");
    delay(4000);
  
}
