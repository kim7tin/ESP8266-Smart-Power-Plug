#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#define MAX_PASS_LEN 16
#define TOKEN_LEN 16

const char *wifi_ssid = "KHome-X";
const char *wifi_password = "HoangTung";
const char *control_password = "123456";
String friendlyName = "Light";
IPAddress ip(192, 168, 3, 66); // where xx is the desired IP Address
IPAddress gateway(192, 168, 3, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

const byte on_message[] = {0x0d, 0x0a, 0x2b, 0x49, 0x50, 0x44, 0x2c, 0x34, 0x3a, 0xa0, 0x01, 0x01, 0xa2 };
const byte off_message[] = {0x0d, 0x0a, 0x2b, 0x49, 0x50, 0x44, 0x2c, 0x34, 0x3a, 0xa0, 0x01, 0x00, 0xa1 };

char code[TOKEN_LEN + 1], token[TOKEN_LEN + 1];
char mac[13];

// on 0d 0a 2b 49 50 44 2c 30 2c 34 3a a0 01 01 a2
// off 0d 0a 2b 49 50 44 2c 30 2c 34 3a a0 01 00 a1

ESP8266WebServer server ( 666 );

const int led = 2;

void handleNotFound() {
  digitalWrite ( led, 1 );
  Serial.print("Not found");
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

void discover() {
  if (!server.hasArg("t")) {
    server.send ( 400, "text/html", "" );
    return;
  }

  char receive_token[TOKEN_LEN + 1];
  server.arg("t").toCharArray(receive_token, TOKEN_LEN + 1);

  Serial.println(receive_token);
  Serial.println(token);

  if (strcmp(receive_token, token) != 0) {
    server.send ( 403, "text/html", "" );
    return;
  }
  String string_mac(mac);
  String info = "{\"endpoints\":[{\"endpointId\":\"" + string_mac + "\",\"friendlyName\":\"" + friendlyName + "\",\"description\":\"Smart Light by Duc Tin\",\"manufacturerName\":\"Duc Tin\",\"displayCategories\":[\"LIGHT\"],\"cookie\":{},\"capabilities\":[{\"type\":\"AlexaInterface\",\"interface\":\"Alexa\",\"version\":\"3\"},{\"type\":\"AlexaInterface\",\"interface\":\"Alexa.PowerController\",\"version\":\"3\",\"properties\":{\"supported\":[{\"name\":\"powerState\"}],\"proactivelyReported\":false,\"retrievable\":false}}]}]}";

  server.send ( 200, "text/html", info );
  digitalWrite ( led, 0 );
}

void control() {
  if (!server.hasArg("t") || !server.hasArg("i") || !server.hasArg("c")) {
    server.send ( 400, "text/html", "" );
    return;
  }

  char receive_token[TOKEN_LEN + 1];
  server.arg("t").toCharArray(receive_token, TOKEN_LEN + 1);

  if (strcmp(receive_token, token) != 0) {
    server.send ( 403, "text/html", "" );
    return;
  }

  char control[2];
  server.arg("c").toCharArray(control, 2);
  Serial.println(control[0]);
  if (control[0] == 49) {
    Serial.println("ON");
    Serial.write(on_message, sizeof(on_message));
  } else if (control[0] == 48) {
    Serial.println("OFF");
    Serial.write(off_message, sizeof(off_message));
  }

  server.send ( 200, "text/html", "" );
  digitalWrite ( led, 0 );
}

void setup ( void ) {
  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  Serial.begin ( 9600 );
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
  WiFi.mode(WIFI_STA);
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
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
  byte byte_mac[6];
  WiFi.macAddress(byte_mac);
  sprintf(mac, "%02X%02X%02X%02X%02X%02X", byte_mac[0], byte_mac[1], byte_mac[2], byte_mac[3], byte_mac[4], byte_mac[5]);
  Serial.println(mac);
  server.on("/login", login);
  server.on("/access", access);
  server.on("/discover", discover);
  server.on("/control", control);

  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}

void loop ( void ) {
  server.handleClient();
}
