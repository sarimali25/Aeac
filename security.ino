#include <SocketIOclient.h>
//#include <WebSockets.h>
#include <WebSocketsClient.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<WiFiManager.h>
#include <WiFiClientSecure.h>
//#include <WebSocketsClient.h>

//#include <SocketIoClient.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
String  auth = "HDKSpW5Qyjdd75R9LckZ5EGVC0a2";
#define SERVER "34.31.163.175"
 
SocketIOclient socketIO;

void messageHandler(uint8_t* payload) {
  StaticJsonDocument<64> doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println(error.f_str());
    return;
  }

  String messageKey = doc[0];
  int button = doc[1];
  int value = doc[2];
  if (messageKey == "buttonState") {
    digitalWrite(button, value);
  }

  String jsonString;
  serializeJson(doc, jsonString);

  // Print the JSON string on the Serial Monitor
  Serial.println("Received JSON: " + jsonString);

}

void joinRoom(){
  StaticJsonDocument<64> doc;
  JsonArray array = doc.to<JsonArray>();
  array.add("join");
  array.add(auth);
  // JSON to String (serialization)
  String output;
  serializeJson(doc, output);
  Serial.println("Sending JSON: " + output);
  socketIO.sendEVENT(output);
  }
void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      Serial.println("Disconnected!");
      
      break;

    case sIOtype_CONNECT:
      Serial.printf("Connected to url: %s%s\n", SERVER, payload);
      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      joinRoom();
      break;

    case sIOtype_EVENT:
      messageHandler(payload);
      break;
  }
}

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";
const char* host = "us-central1-aeac-binz.cloudfunctions.net";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "04 d1 e7 6f 70 29 f2 93 44 13 34 cd 44 a4 c1 18 2e 12 c0 64 c5 d6 64 e4 b0 3c 6b 29 d4 ce 65 8b 39 26 d9 5d e0 3c 57 7b d6 95 28 48 48 c7 9b b4 0c c6 3c 95 eb 2f 94 84 0d c2 dd fa 19 83 ae c4 2f";

int flag=1;

void setup()
{
  // Debug console
  Serial.begin(9600);
  pinMode(D0,OUTPUT);
  digitalWrite(D0,HIGH);
  pinMode(D1,OUTPUT);
  digitalWrite(D1,HIGH);
  pinMode(D2,OUTPUT);
  digitalWrite(D2,HIGH);
  pinMode(0,OUTPUT);
  digitalWrite(0,LOW);
  pinMode(2, INPUT_PULLUP);
  pinMode(D6,OUTPUT);
  digitalWrite(D6,HIGH);  
  pinMode(D7,OUTPUT);
  digitalWrite(D7,HIGH);  
  pinMode(D5,OUTPUT);
  digitalWrite(D5,HIGH);

  WiFiManager wifimanager;
  wifimanager.autoConnect("AEAC");
  
  Serial.println(WiFi.localIP());
   // server address, port and URL
  socketIO.begin(SERVER, 4001,"/socket.io/?EIO=4");
  
  socketIO.onEvent(socketIOEvent);
  digitalWrite(2,HIGH); 
}

void loop()
{
//  sensor();
  socketIO.loop();
}


void sensor(){
  int sensorVal = digitalRead(2);
  //print out the value of the pushbutton
  Serial.println(sensorVal);
  if (sensorVal == LOW && digitalRead(0)==LOW) {
    flag=0;
    digitalWrite(D0,HIGH);
  } else if (sensorVal == HIGH && flag==0 && digitalRead(0)==LOW){
    flag=1;
    digitalWrite(D0,LOW);
    httpRequest();
  }
}
void httpRequest(){
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }


//  if (client.verify(fingerprint, host)) {
//    Serial.println("certificate matches");
//  } else {
//    Serial.println("certificate doesn't match");
//  }

  String url = "/addMessage?id="+String(auth)+"&msg=1";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}
