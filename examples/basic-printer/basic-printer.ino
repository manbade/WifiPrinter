#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <wifi.h>
#else
#error "only compatible with ESP8266 or ESP32"
#endif
#include <WifiPrinter.h>

const char* ssid     = "XXXX";
const char* password = "XXXX";

const short port = 9000;
WifiPrinter myWifiPrinter(port);


void setup(){
  Serial.begin(115200);
  Serial.println(F("Connecting to Wifi..."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  myWifiPrinter.begin();
  myWifiPrinter.enableReset(true);

  Serial.print(F("Wifi connected, please switch over to telnet on port "));
  Serial.println(port);
}

void loop(){

  static unsigned long lastMillis = 0;
  if(millis() - lastMillis > 10 * 1000UL){
    lastMillis = millis();
    unsigned int freeHeap = ESP.getFreeHeap();
    myWifiPrinter.print(F("Free heap size: "));
    myWifiPrinter.println(String(freeHeap));
    Serial.print(F("Free heap size: "));
    Serial.println(String(freeHeap));
  }

  myWifiPrinter.loop();
}
