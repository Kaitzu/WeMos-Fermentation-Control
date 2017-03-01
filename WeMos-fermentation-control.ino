// STC-1000 functionality with browser control 
// Usage: http://<ip>/?box=<set temp> Example: http://192.168.100.16/?box=23

#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <ESP8266HTTPClient.h>
 
// OneWire DS18S20, DS18B20, DS1822 Temperature init
 
OneWire  ds(D4);  // on pin D4 (a 4.7K resistor is necessary)

const char* ssid = "xxx"; // Your WiFi SSID
const char* password = "xxx"; // Your WiFi Password
const int Relay = D5;
const int Relay2 = D6;
int heatStatus = 0;
int fridgeStatus = 0;
int ledPin = D5;
WiFiServer server(80);

float value = 20.00; // Temperature default value
float tolerance = 00.30; // Temerature setup value
String box = "20"; // Temperature default value for browser
int timeToLog = 0; // Cloud logger count


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
      // logCloud(); // Uncomment and modify function for your needs
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
  // Send temperature reading to cloud server
  float temperature = getTemp(); //will take about 750ms to run
  String temp = String(temperature);
   HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
        http.begin("http://192.168.100.99/log.php?t=" + temp);

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
}



