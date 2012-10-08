/*
  Ludikenergie tachometer v0.3
 
 A web server that shows the number of spins since Arduino started
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * SIGNAL and GND from bicycle jack cable to pins 2 and GND
 * Optional : 8-ohm speaker on digital pin 8
 * Optional : LED attached from pin 9 to ground
 
 created 19 Jul 2012
 updated 22 Aug 2012
 by Baptiste Gaultier (Telecom Bretagne)
 
 This code is in the public domain.
 
 */

#include <SPI.h>
#include <Ethernet.h>


// Enter a MAC address and IP address for your controller below.
// Newer Arduino include a sticker with the device's MAC address
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE5, 0x25 };
// each bike needs a unique IP address and are normally assigned DHCP
// if DHCP fails, this IP address will be used :
IPAddress ip(192, 168, 1, 1);


// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// Store the number of revolutions of the wheel since Arduino started
long spins = 0;

// Store the last time we increment spins
long previousMillis = 0;       

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 200;           // interval at which to increment

// Variables will change:
int currentState; // current state of the wheel sensor
int lastState = 0;  // previous state of the wheel sensor
int randNumber; // random number


void setup() {
 // Open serial communications and wait for port to open:
 Serial.begin(9600);
 
 //configure pin2 as an input and enable the internal pull-up resistor
 pinMode(2, INPUT_PULLUP);
 pinMode(9, OUTPUT); 
  
 // welcome message
 Serial.println("Ludikenergie tachometer v0.3 starting...");
 
 // attempt a DHCP connection:
 Serial.println("Attempting to get an IP address using DHCP:");
 if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Serial.println("failed to get an IP address using DHCP, trying manually");
    // please change the value of ip for each bike, subnet defaults to 255.255.255.0 
    Ethernet.begin(mac, ip);
  }
  
  // start listening for clients
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  
  // if analog input pin 5 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(5));
}


void loop() {
  //read the wheel sensor value into a variable
  int currentState = digitalRead(2);
 
  // Keep in mind the pullup means the sensor's logic is
  // inverted. It goes HIGH when it's open,and LOW when
  // it's not. Turn on pin 9 when the  wheel sensor is
  // close to the magnet, and off when it's not:
  if(currentState != lastState) {
    if (currentState == HIGH) {
      digitalWrite(9, LOW);
      lastState = HIGH;
    }
    else {
      // Revolution of the wheel
      spins ++;
      digitalWrite(9, HIGH);
      tone(8, 440, 20);
      Serial.print("spins : ");
      Serial.println(spins);
      lastState = LOW;
    }
  }
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          // MIME type "application/json" http://www.ietf.org/rfc/rfc4627.txt
          client.println("Content-Type: application/json");
          client.println("Connnection: close");
          client.println();
          client.print("{\"spins\":\"");
          client.print(spins);
          client.print("\"}");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

