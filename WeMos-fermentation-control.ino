// WeMos-Fermentation-Control
// Control your fermentation temperature with web interface and WiFi and log temperature reading to cloud server 
// Set temp using following url: http://<IP FROM SERIAL MONITOR>/?box=<TEMP>

#include <ESP8266WiFi.h>
#include <OneWire.h>
 
// Wifi and cloud server
const char* ssid = "xxx";                         // SSID
const char* password = "xxx";                     // WiFi password
const char* host = "xxx";                         // Temperature logger host
const char* privateKey = "xxx";                   // Secret key for logger host

OneWire  ds(D4);                                  // OneWire DS18S20 on pin D4 (a 4.7K resistor is necessary)

// Pins
const int Relay = D5;
const int Relay2 = D6;
const int ledPin = D5;

// Variables
int heatStatus = 0;                               // Set heat 0
int fridgeStatus = 0;                             // Set cool 0
int timeToLog = 495;                              // Cloud logger count. 5 measurements before first log
float value = 20.00;                              // Temperature default value
float tolerance = 00.30;                          // Temerature setup value
String box = "20";                                // Temperature default value for browser

// Server setup
WiFiServer server(80);


void setup() {
  Serial.begin(115200);
  delay(10);
 
  pinMode(Relay, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");  
  
}
 
void loop() {
    
  // Read temperature
  float temperature = getTemp(); //will take about 750ms to run

  // Read status of relays
  heatStatus = digitalRead(Relay);
  fridgeStatus = digitalRead(Relay2);
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
        Serial.println("new client");
        // Read the first line of the request
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();
        // Read box value
        if (request.indexOf("/?box=") != -1) {
          box = request.substring(10,12);
          value = box.toFloat();
        }
        // Return the http response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.print("<head>");
        client.print("<meta http-equiv=\"refresh\" content=\"2\">");
        client.print("<TITLE />Fermentation fridge</title>");
        client.print("</head>");
        client.print("SET:");
        client.print(value);
        client.println("<br>");
        client.print("TEMP:");
        client.print(temperature);
        client.println("<br>");
        client.print("HEATER STATUS:");
        client.print(heatStatus);
        client.println("<br>");
        client.print("FRIDGE STATUS:");
        client.print(fridgeStatus);
        client.println("</html>");
       
        delay(1);
        client.stop();
        Serial.println("Client disconnected");
        Serial.println("");
        } 
     } 
  }

  // Temperature control loop
  Serial.print("Temp: ");
  Serial.println(temperature);
  Serial.print("SET Temp: ");
  Serial.println(value);

  if (temperature <= value - tolerance) {
      Serial.println("HEATER ON");
      digitalWrite(ledPin, HIGH);
      digitalWrite(Relay2, LOW); // Turns OFF Relay 2
      digitalWrite(Relay, HIGH); // Turns ON Relay 1
  }

  if (temperature >= value + tolerance) {
      Serial.println("FREEZE ON");
      digitalWrite(ledPin, LOW);
      digitalWrite(Relay, LOW); // Turns OFF Relay 1
      digitalWrite(Relay2, HIGH); // Turns OFF Relay 2
  }

  if (timeToLog >= 500) {
      // Send Temperature reading to Cloud.
      logCloud();
      timeToLog = 0;
  }

  timeToLog++;
  Serial.println(timeToLog);
  delay(250);

}

float getTemp(){
  
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
  
  delay(750); // Wait for temperature conversion to complete

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}

void logCloud() {

  // Read temperature
  float temperature = getTemp(); //will take about 750ms to run
  String temp = String(temperature);

  // Send temperature reading to cloud server
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/log.php";
  url += "?temp=";
  url += temp;
  url += "&key=";
  url += privateKey;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}



