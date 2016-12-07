// Includes
#include <Time.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <FS.h>
#include <SerialCommand.h>
#include "IthoCC1101.h"

SerialCommand sCmd;
IthoCC1101 rf;
IthoPacket packet;

String Version = "0.6";

// Div
File UploadFile;
String fileName;
int FSTotal;
int FSUsed;


// WIFI
String ssid    = "SSID";
String password = "PASS";
String espName    = "Itho";

// webserver
ESP8266WebServer  server(80);
MDNSResponder   mdns;
WiFiClient client;


String ClientIP;


String header       =  "<html lang='en'><head><title>Itho control panel</title><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' href='http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css'><script src='https://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js'></script><script src='http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/js/bootstrap.min.js'></script></head><body>";
String navbar       =  "<nav class='navbar navbar-default'><div class='container-fluid'><div class='navbar-header'><a class='navbar-brand' href='/'>Itho control panel</a></div><div><ul class='nav navbar-nav'><li><a href='/'><span class='glyphicon glyphicon-question-sign'></span> Status</a></li><li class='dropdown'><a class='dropdown-toggle' data-toggle='dropdown' href='#'><span class='glyphicon glyphicon-cog'></span> Tools<span class='caret'></span></a><ul class='dropdown-menu'><li><a href='/updatefwm'>Firmware</a></li><li><a href='/api?action=restart'>Restart</a></ul></li><li><a href='https://github.com/incmve/Itho-WIFI-remote' target='_blank'><span class='glyphicon glyphicon-question-sign'></span> Help</a></li></ul></div></div></nav>  ";

String containerStart   =  "<div class='container'><div class='row'>";
String containerEnd     =  "<div class='clearfix visible-lg'></div></div></div>";
String siteEnd        =  "</body></html>";

String panelHeaderName    =  "<div class='col-md-4'><div class='page-header'><h1>";
String panelHeaderEnd   =  "</h1></div>";
String panelEnd       =  "</div>";

String panelBodySymbol    =  "<div class='panel panel-default'><div class='panel-body'><span class='glyphicon glyphicon-";
String panelBodyName    =  "'></span> ";
String panelBodyValue   =  "<span class='pull-right'>";
String panelcenter   =  "<div class='row'><div class='span6' style='text-align:center'>";
String panelBodyEnd     =  "</span></div></div>";

String inputBodyStart   =  "<form action='' method='POST'><div class='panel panel-default'><div class='panel-body'>";
String inputBodyName    =  "<div class='form-group'><div class='input-group'><span class='input-group-addon' id='basic-addon1'>";
String inputBodyPOST    =  "</span><input type='text' name='";
String inputBodyClose   =  "' class='form-control' aria-describedby='basic-addon1'></div></div>";
String ithocontrol     =  "<a href='/api?action=Low'<button type='button' class='btn btn-default'> Low</button></a><a href='/api?action=Medium'<button type='button' class='btn btn-default'> Medium</button></a><a href='/api?action=High'<button type='button' class='btn btn-default'> High</button><a href='/api?action=Timer'<button type='button' class='btn btn-default'> Timer</button></a></a><br><a href='/api?action=Learn'<button type='button' class='btn btn-default'> Learn</button></a></div>";



// ROOT page
void handle_root()
{
  // get IP
  IPAddress ip = WiFi.localIP();
  ClientIP = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  delay(500);

  String title1     = panelHeaderName + String("Itho WIFI remote") + panelHeaderEnd;
  String IPAddClient    = panelBodySymbol + String("globe") + panelBodyName + String("IP Address") + panelBodyValue + ClientIP + panelBodyEnd;
  String ClientName   = panelBodySymbol + String("tag") + panelBodyName + String("Client Name") + panelBodyValue + espName + panelBodyEnd;
  String ithoVersion   = panelBodySymbol + String("tag") + panelBodyName + String("Version") + panelBodyValue + Version + panelBodyEnd;
  String Uptime     = panelBodySymbol + String("time") + panelBodyName + String("Uptime") + panelBodyValue + hour() + String(" h ") + minute() + String(" min ") + second() + String(" sec") + panelBodyEnd + panelEnd;


  String title3 = panelHeaderName + String("Commands") + panelHeaderEnd;

  String commands = panelBodySymbol + panelBodyName + panelcenter + ithocontrol + panelBodyEnd;


  server.send ( 200, "text/html", header + navbar + containerStart + title1 + IPAddClient + ClientName + ithoVersion + Uptime + title3 + commands + containerEnd + siteEnd);
}


// Setup
void setup(void)
{
  Serial.begin(115200);
  WiFi.hostname(espName);
  // Check if SPIFFS is OK

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS failed, needs formatting");
    handleFormat();
    delay(500);
    ESP.restart();
  }
  else
  {
    FSInfo fs_info;
    if (!SPIFFS.info(fs_info))
    {
      Serial.println("fs_info failed");
    }
    else
    {
      FSTotal = fs_info.totalBytes;
      FSUsed = fs_info.usedBytes;
    }
  }
  //mySerial.begin(115200);  // uncomment this line to use SoftSerial
  WiFi.begin(ssid.c_str(), password.c_str());
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 31)
  {
    delay(1000);
    Serial.print(".");
    ++i;
  }
  if (WiFi.status() != WL_CONNECTED && i >= 30)
  {
    WiFi.disconnect();
    delay(1000);
    Serial.println("");
    Serial.println("Couldn't connect to network :( ");
    Serial.println("Review your WIFI settings");

  }
  else
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(espName);

  }
  delay(500);
  Serial.println("setup begin");
  rf.init();
  Serial.println("setup done");
  sendRegister(); // Your Itho only accepts this command while in learning mode, powercycle your itho then start the nodemcu.
  Serial.println("join command sent");
  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("High",    sendFullSpeed);
  sCmd.addCommand("Medium",     sendMediumSpeed);
  sCmd.addCommand("Low",   sendLowSpeed);
  sCmd.addCommand("Timer", sendTimer);
  sCmd.addCommand("Learn", sendRegister);        // Register remote in ithon fan
  sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")

  server.on ( "/format", handleFormat );
  server.on("/", handle_root);

  server.on("/api", handle_api);
  server.on("/updatefwm", handle_updatefwm_html);
  



  // Upload firmware:
  server.on("/updatefw2", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []()
  {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
      fileName = upload.filename;
      Serial.setDebugOutput(true);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
      {
        Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
      if (Update.end(true)) //true to set the size to the current progress
      {
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      }
      else
      {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);

    }
    yield();
  });



  if (!mdns.begin(espName.c_str(), WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);
  rf.initReceive();
}




// LOOP
void loop(void)
{
  sCmd.readSerial();
  server.handleClient();


    if (rf.checkForNewPacket()) {
      packet = rf.getLastPacket();
      //show counter
      Serial.print("counter=");
      Serial.print(packet.counter);
      Serial.print(", ");
      //show command
      switch (packet.command) {
        case IthoUnknown:
          Serial.print("unknown\n");
          break;
        case IthoLow:
          Serial.print("low\n");
          break;
        case IthoMedium:
          Serial.print("medium\n");
          break;
        case IthoFull:
          Serial.print("full\n");
          break;
        case IthoTimer1:
          Serial.print("timer1\n");
          break;
        case IthoTimer2:
          Serial.print("timer2\n");
          break;
        case IthoTimer3:
          Serial.print("timer3\n");
          break;
        case IthoJoin:
          Serial.print("join\n");
          break;
        case IthoLeave:
          Serial.print("leave\n");
          break;
      } // switch (recv) command
     // checkfornewpacket
  yield();
  }
}


// VOID setups

void handle_api()
{
  // Get vars for all commands
  String action = server.arg("action");
  String value = server.arg("value");
  String api = server.arg("api");

  if (action == "High")
  {
    sendFullSpeed();
    server.send ( 200, "text/html", "Full Powerrr!!!");
  }

  if (action == "Medium")
  {
    sendMediumSpeed();
    server.send ( 200, "text/html", "Medium speed selected");
  }

  if (action == "Low")
  {
    sendLowSpeed();
    server.send ( 200, "text/html", "Slow speed selected");
  }

  if (action == "Timer")
  {
    sendTimer();
    server.send ( 200, "text/html", "Timer on selected");
  }

  if (action == "Learn")
  {
    sendRegister();
    server.send ( 200, "text/html", "Send learn command OK");
  }


  if (action == "reset" && value == "true")
  {
    server.send ( 200, "text/html", "Reset ESP OK");
    delay(500);
    Serial.println("RESET");
    ESP.restart();
  }
}

void handle_updatefwm_html()
{
  server.send ( 200, "text/html", "<form method='POST' action='/updatefw2' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form><br<b>For firmware only!!</b>");
}






void handle_update_upload()
{
  if (server.uri() != "/update2") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.setDebugOutput(true);
    Serial.printf("Update: %s\n", upload.filename.c_str());
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) { //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    Serial.setDebugOutput(false);
  }
  yield();
}
void handle_update_html2()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}

void handleFormat()
{
  server.send ( 200, "text/html", "OK");
  Serial.println("Format SPIFFS");
  if (SPIFFS.format())
  {
    if (!SPIFFS.begin())
    {
      Serial.println("Format SPIFFS failed");
    }
  }
  else
  {
    Serial.println("Format SPIFFS failed");
  }
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS failed, needs formatting");
  }
  else
  {
    Serial.println("SPIFFS mounted");
  }
}

void sendRegister() {
  Serial.println("sending join...");
  rf.sendCommand(IthoJoin);
  Serial.println("sending join done.");
  handle_root();
}

void sendLowSpeed() {
  Serial.println("sending low...");
  rf.sendCommand(IthoLow);
  Serial.println("sending low done.");
  handle_root();
}

void sendMediumSpeed() {
  Serial.println("sending medium...");
  rf.sendCommand(IthoMedium);
  Serial.println("sending medium done.");
  handle_root();
}

void sendFullSpeed() {
  Serial.println("sending FullSpeed...");
  rf.sendCommand(IthoFull);
  Serial.println("sending FullSpeed done.");
  handle_root();
}

void sendTimer() {
  Serial.println("sending timer...");
  rf.sendCommand(IthoTimer1);
  Serial.println("sending timer done.");
  handle_root();
}

void unrecognized(const char *command) {
  Serial.println("What?");
}