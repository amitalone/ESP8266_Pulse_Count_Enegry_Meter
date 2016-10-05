#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "XXX";
const char* password = "XXXX";

const char* data_server_url = "//etrack/logger";


ESP8266WebServer server(80);

String data ="";
String sessionID = "-1";
  
void handleRoot() {
  String header ="HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:*\r\nAccess-Control-Allow- Methods:GET, PUT \r\n";
header += "Access-Control-Allow-Headers *AUTHORISED*\r\nContent-type: text/html\r\nServer: ESP8266-1\r\n\n";

  int cmd = server.arg("cmd").toInt();
   
  if(cmd == 2) {
    Serial.write("3");
    delay(50);
    refreshLast();
    server.sendContent(header+data);  
  }
  
 // server.send(200, "text/plain", data); 
  
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  data = "NONE";
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  //   Serial.print(".");
  }
 /*   Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  */

  if (MDNS.begin("esp8266")) {
    //Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
 // Serial.println("HTTP server started");
 String msg = "{\"r\":\"ip\",\"ip\":\"";
 msg = msg + WiFi.localIP().toString() + "\",\"ap\":\"" + ssid +"\"}";
 
 String body = postData(msg);
 int k = body.toInt();
 if(k > 0) {
  sessionID = k +"";
 }
}

String serialData ="";
void loop(void){
  server.handleClient();
  if (Serial.available()) {
    serialData = Serial.readString();
    serialData.trim(); 
    String cmd = serialData.substring(6,8);
  //  Serial.println(cmd);
   if(cmd =="lc") {
    data = serialData;
   }
   if(cmd =="rc") {
    serialData.concat("sid:\"");
    serialData.concat(sessionID);
    serialData.concat("\"}");
    postData(serialData);
   }
  }
  if(WiFi.status() == WL_CONNECTED) 
  {
   repeatPost(300); 
  }
  
}

void refreshLast() {
  if(String("NONE") != data) {
    postData(data);
  }
}

long startTime = millis();
long oldV = 0;
void repeatPost(long seconds) {
  seconds = seconds * 1000;
  long d = millis() - startTime;
  if(d %  seconds == 0) {
    if(oldV != d) {
       // FUNCTION HERE
       refreshLast();
      oldV = d;
    }
  }
}
String postData(String data) {
  String payload ="";
  HTTPClient http;
  http.begin(data_server_url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String s = "j=";
  s.concat(data);
  http.POST(s);
  //http.writeToStream(&Serial);
 // Serial.println(s);
 int httpCode = http.GET();
 if(httpCode > 0) {
  if(httpCode == HTTP_CODE_OK) {
   payload = http.getString();
  }
 }
  http.end();
  return payload;
}

