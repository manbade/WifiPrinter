#include "TelnetPrinter.h"

WiFiServer telnetServer(23);
WiFiClient serverClient;
String bufferPrint = "";


void TelnetPrinter::begin() {
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  bufferPrint.reserve(100);
}


void TelnetPrinter::stop () {
  if (serverClient && serverClient.connected()) serverClient.stop();
  telnetServer.stop();
}


void TelnetPrinter::loop() {
  //Handle connections
  if (telnetServer.hasClient()) {
    if (!serverClient || !serverClient.connected()) {
      if (serverClient) serverClient.stop(); //stop second client, only 1 supported
      serverClient = telnetServer.available();
      serverClient.flush(); //start with clean sheet
      showInfo();
      //empty input buffer
      while(serverClient.available()) {
        serverClient.read();
      }
    }
  }
  //Loop through activities
  _connected = (serverClient && serverClient.connected());
  if (_connected) {
    //check if commands has been sent
    while(serverClient.available()) {
      char character = serverClient.read();
      if (character == '\n' || character == '\r') {
        //Commands end with <newline> --> handle command
        if (_command.length() > 0) {
          handle();
        }
        _command = ""; // Init for next command
      } else if (isPrintable(character)) {
        // Concat
        _command.concat(character);
      }
    }
  }
}


// Allow ESP reset over telnet client
void TelnetPrinter::enableReset(boolean enable){
    _resetCommandEnabled = enable;
}


boolean TelnetPrinter::isActive() {
    return _connected;
}


size_t TelnetPrinter::write(uint8_t character) {
  // New line writted before ?
  if (_newLine) {
    String show = "";
    // Show time in millis
    if (_showTime) {
      show.concat ("(t: ");
      show.concat (millis());
      show.concat ("ms)");
      if (_connected) {  // send data to Client
        bufferPrint = show;
      }
    }
    _newLine = false;
  }

  // Print ?
  boolean doPrint = false;

  // New line ?
  if (character == '\n') {
    bufferPrint.concat("\r");
    _newLine = true;
    doPrint = true;
  } else if (bufferPrint.length() == 100) { // Limit of buffer
    doPrint = true;
  }

  // Write to telnet Buffered
  bufferPrint.concat((char)character);

  // Send the characters buffered by print.h
  if (doPrint) { // Print the buffer
    // Write to telnet Buffered
    if (_connected) {  // send data to Client
      serverClient.print(bufferPrint);
    }
    // Empty the buffer
    bufferPrint = "";
    return 1;
  } else return 1;
}

////// Private

// Show help of commands
void TelnetPrinter::showInfo() {
    // Show the initial message
    String help = "";
    help.concat(F("*** ESP8266 logger***\r\n"));
    help.concat(F(" * IP:"));
    help.concat(WiFi.localIP().toString());
    help.concat(F("\r\n * Mac address:"));
    help.concat(WiFi.macAddress());
    help.concat(F("\r\n * Free Heap RAM: "));
    help.concat(ESP.getFreeHeap());
    help.concat(F("\r\n**************************************\r\n"));
    help.concat(F(" * Commands:\r\n"));
    help.concat(F("    ? or help -> display these help of commands\r\n"));
    help.concat(F("    q -> quit (close this connection)\r\n"));
    help.concat(F("    m -> display memory available\r\n"));
    if (_resetCommandEnabled) {
        help.concat(F("    reset -> reset the ESP8266\r\n"));
    }
    help.concat(F("\r\n* Please type the command and press enter to execute.(? or h for this help)\r\n\r\n"));
    serverClient.print(help);
}


// Process user command over telnet
void TelnetPrinter::handle() {
    serverClient.print(F("Command recevied: "));
    serverClient.println(_command);
    String options = "";
    uint8_t pos = _command.indexOf(" ");
    if (pos > 0) {
        options = _command.substring (pos+1);
    }
    // Process the command
    if (_command == "h" || _command == "?") {
        // Show help
        showInfo();
    } else if (_command == "q") {
        // Quit
        serverClient.println(F("Closing telnet connection ..."));
        serverClient.stop();
    } else if (_command == "m") {
        serverClient.print(F("* Free Heap RAM: "));
        serverClient.println(ESP.getFreeHeap());
    }  else if (_command == "t") {
        // Show time
        _showTime = !_showTime;
        serverClient.printf("* Show time: %s\r\n", (_showTime)?"On":"Off");
    }  else if (_command == "reset" && _resetCommandEnabled) {
        serverClient.println(F("* Reset ..."));
        serverClient.println(F("* Closing telnet connection ..."));
        serverClient.println(F("* Resetting the ESP8266 ..."));
        serverClient.stop();
        telnetServer.stop();
        delay (500);
        // Reset
        ESP.reset();
    }
}
