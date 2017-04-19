#include <TelnetPrinter.h>

TelnetPrinter debug;

void setup(){
  Serial.begin(115200);

  debug.begin();
  debug.enableReset(true);

}

void loop(){
  static unsigned long lastMillis = 0;
  if(millis() - lastMillis > 10 * 1000){
    unsigned int freeHeap = ESP.getFreeHeap();
    debug.print(F("Free heap size: "));
    debug.println(String(freeHeap));
    lastMillis = millis();
  }
}
