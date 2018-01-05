#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#define MAX_PASS_LEN 16
#define TOKEN_LEN 16

const char *wifi_ssid = "KHome";
const char *wifi_password = "HoangTung";
const char *control_password = "12345";

char code[TOKEN_LEN], token[TOKEN_LEN];

ESP8266WebServer server ( 666 );

const int led = 2;

void handleRoot() {
  digitalWrite ( led, 1 );
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 400,

             "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

             hr, min % 60, sec % 60
           );
  server.send ( 200, "text/html", temp );
  digitalWrite ( led, 0 );
}

void handleNotFound() {
  digitalWrite ( led, 1 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  digitalWrite ( led, 0 );
}


void login() {
  if (!server.hasArg("p")) {
    server.send ( 400, "text/html", "" );
    return;
  }

  char receive_password[MAX_PASS_LEN + 1];
  server.arg("p").toCharArray(receive_password, MAX_PASS_LEN + 1);

  if (strcmp(receive_password, control_password) != 0) {
    server.send ( 403, "text/html", "" );
    return;
  }

  String creat_code;
  for (int i = 0; i < TOKEN_LEN; i++) {
    code[i] = random(97, 122);
    creat_code += String(code[i]);
    EEPROM.write(i, code[i]);
  }
  EEPROM.commit();
  server.send ( 200, "text/html", creat_code );
}

void access() {
  if (!server.hasArg("c")) {
    server.send ( 400, "text/html", "" );
    return;
  }

  char receive_code[TOKEN_LEN + 1];
  server.arg("c").toCharArray(receive_code, TOKEN_LEN + 1);

  if (strcmp(receive_code, code) != 0) {
    server.send ( 403, "text/html", "" );
    return;
  }

  String creat_token;
  for (int i = 0; i < TOKEN_LEN; i++) {
    token[i] = random(97, 122);
    creat_token += String(token[i]);
    EEPROM.write(TOKEN_LEN + i, token[i]);
  }
  EEPROM.commit();
  server.send ( 200, "text/html", creat_token );
}

void discover(){
  if (!server.hasArg("t")) {
    server.send ( 400, "text/html", "" );
    return;
  }

  char receive_token[TOKEN_LEN + 1];
  server.arg("t").toCharArray(receive_token, TOKEN_LEN + 1);

  if (strcmp(receive_token, token) != 0) {
    server.send ( 403, "text/html", "" );
    return;
  }

  
  digitalWrite ( led, 1 );
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 400,

             "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

             hr, min % 60, sec % 60
           );
  server.send ( 200, "text/html", temp );
  digitalWrite ( led, 0 );
}

void control(){
  if (!server.hasArg("t")) {
    server.send ( 400, "text/html", "" );
    return;
  }

  char receive_token[TOKEN_LEN + 1];
  server.arg("t").toCharArray(receive_token, TOKEN_LEN + 1);

  if (strcmp(receive_token, token) != 0) {
    server.send ( 403, "text/html", "" );
    return;
  }

  server.send ( 200, "text/html", "" );
  digitalWrite ( led, 0 );
}

void setup ( void ) {
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  Serial.begin ( 115200 );
  EEPROM.begin(512);


  Serial.println("EPPROM:");

  for (int i = 0; i < TOKEN_LEN; i++) {
    char value = EEPROM.read(i);
    code[i] = value;
    value =  EEPROM.read(TOKEN_LEN + i);
    token[i] = value;
  }

  Serial.println(code);
  Serial.println(token);

  Serial.println("//////////////////////////////");



  // Static local IP for config NAT in router

  IPAddress ip(192, 168, 1, 66); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);

  WiFi.begin ( wifi_ssid, wifi_password );
  Serial.println ( "" );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( wifi_ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( "esp8266" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on("/login", login);
  server.on("/access", access);
  server.on("/discover", discover);
  server.on("/control", control);


  server.on ( "/", handleRoot );
  server.on ( "/test.svg", drawGraph );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}

void loop ( void ) {
  server.handleClient();
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send ( 200, "image/svg+xml", out);
}
