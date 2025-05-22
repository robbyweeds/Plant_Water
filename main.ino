#include <arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// Potentiometer is connected to GPIO 34
const int potPin = 34;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET  -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



int pumpstate = 0;
int pumpAuto = 0;

// Replace with your network credentials
const String ssid     = "ESP32-Access-Point";
const String password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;
int led2 = 2;

String output26State;
String output27State;

// Variable to store the HTTP request
String header;

// variable for storing the potentiometer value
int potValue = 0;

// Pump Relay Pin
int pumpRelayPin = 23;


void setup() {
  Serial.begin(115200);
  Wire.begin();
    Serial.println("SCANNER VALUE");
    Serial.println("\nI2C Scanner");
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);






















//  Initialize pump run relay
    pinMode(pumpRelayPin, OUTPUT);
    digitalWrite(pumpRelayPin, LOW);

// Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
// Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

// Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
// Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();

  delay(1000);


}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  //  Read Sensor
    potValue = analogRead(potPin);
    Serial.println("value: " + potValue);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("value: " + potValue);
  display.display(); 


  if (client) {                             // If a new client connects,

    Serial.println(potValue);
    // Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        // Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
 // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /pump/1") >= 0) {
                Serial.println("Pump Start");
                pumpstate = 1;
                digitalWrite(pumpRelayPin, HIGH);
            } else if (header.indexOf("GET /pump/0") >= 0) {
                Serial.println("Pump Off");
                pumpstate = 0;
                digitalWrite(pumpRelayPin, LOW);
            // } else if (header.indexOf("GET /pumpset/1") >= 0) {
            //     Serial.println("Pump set to AUTO");
            //     pumpAuto = 1;
            // } else if (header.indexOf("GET /pumpset/0") >= 0) {
            //     Serial.println("Pump AUTO disengaged");
            //     pumpAuto = 0;
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Moisture Sensor</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
            // Display Current Moisture Reading
            client.println("<p>Moisture Sensor Reading  </p>");
            client.println(potValue);
            
            // PUMP START and STOP
            if (pumpstate == 0) {
                client.println("<p><a href=\"/pump/1\"><button class=\"button\">TURN PUMP ON</button></a></p>");
            } else {
               client.println("<p><a href=\"/pump/0\"><button class=\"button button2\">TURN PUMP OFF</button></a></p>"); 
            }

            // Set to Pump AUTO
            // if (pumpAuto == 0) {
            //     client.println("<p><a href=\"/pumpset/1\"><button class=\"button\">Set to AUTO</button></a></p>");
            // } else if (pumpAuto == 1 && potPin > 1600) {
            //     client.println("<p>Pump on AUTO and RUNNING</p>"); 
            //     client.println("<p><a href=\"/pumpset/0\"><button class=\"button\">Turn Off AUTO</button></a></p>"); 
            // } else if (pumpAuto == 1 && potPin <= 1600) {
            //     client.println("<p>Pump on AUTO and OFF</p>"); 
            //     client.println("<p><a href=\"/pumpset/0\"><button class=\"button\">Turn Off AUTO</button></a></p>"); 
            // } else {
            //    client.println("<p><a href=\"/pumpset/0\"><button class=\"button\">Turn Off AUTO</button></a></p>"); 
            // }

               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  delay(500);
}