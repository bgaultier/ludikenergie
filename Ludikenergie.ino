/*
  Ludikenergie tachometer v0.5
 
 A web client sending time period of wheel revolution of a stationary bike 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * SIGNAL and GND from bicycle jack cable to pins 2 and GND
 * Optional : 8-ohm speaker on digital pin 8
 * Optional : LED attached from pin 9 to ground
 
 created 19 Jul 2012
 updated 09 Oct 2012
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
IPAddress ip(192, 168, 10, 205);

// Server IP address (you must assign a static IP address for the server)
IPAddress server(192, 168, 10, 80);

// initialize the library instance:
EthernetClient client;

// bike identifier
int bikeID = 0;

// used to measure time and period
unsigned long pulseTime,lastTime, period;

// Variables will change:
int currentState; // current state of the wheel sensor
int lastState = 0;  // previous state of the wheel sensor
int readings = 0; // number of pulses read since the last packet was sent
boolean lastConnected = false; // state of the connection last time through the main loop
boolean receiving = false; // are we receiving a packet ?

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  //configure pin2 as an input and enable the internal pull-up resistor
  pinMode(2, INPUT_PULLUP);
  
  // speaker on digital pin 8
  //pinMode(8, OUTPUT);
  // built-in LED
  pinMode(9, OUTPUT);
  
  // welcome message
  Serial.println("Ludikenergie tachometer v0.5 starting...");
  
  // give the ethernet module time to boot up
  delay(1000);
  // start the Ethernet connection
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // DHCP failed, so use a fixed IP address
    Ethernet.begin(mac, ip);
  }
  Serial.print("My IP:");
  Serial.println(Ethernet.localIP());
  
  ip = Ethernet.localIP();
  bikeID = ip[3];
  Serial.print("I'm bike-");
  Serial.println(bikeID);
}


void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    /*char c = client.read();
    Serial.print(c);*/
    client.flush();
    receiving = true;
  }
  
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("Disconnecting.");
    client.stop();
    receiving = false;
  }
  
  // be sure that we are not receiving a packet
  if(!receiving) {
    // read the wheel sensor value into a variable
    int currentState = digitalRead(2);
    
    // Keep in mind the pullup means the sensor's logic is
    // inverted. It goes HIGH when it's open,and LOW when
    // it's not. Turn on pin 9 when the  wheel sensor is
    // close to the magnet, and off when it's not:
    if(currentState != lastState) {
      if (currentState == HIGH) {
        digitalWrite(9, LOW); // turn the LED off
        lastState = HIGH;
      }
      else {
        // Revolution of the wheel
        lastTime = pulseTime;
        pulseTime = millis();
        
        period = pulseTime - lastTime;
        
        if(period > 100) {
          // avoid false positive if period is less than 100ms
          digitalWrite(9, HIGH); // flash LED
          //tone(8, 440, 20); // beep
          delay(20);
          digitalWrite(9, LOW); // turn the LED off
          Serial.print("period : ");
          Serial.println(period);
          lastState = LOW;
          readings++;
          if(readings >= 2) {
            readings = 0;
            sendValue(period); // send period to the server
          }
        }
      }
    }
  }
  
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void sendValue(long period) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    Serial.print("GET /api/post?json={bike-");
    Serial.print(bikeID);
    Serial.print(":");
    Serial.print(period);
    Serial.println("} HTTP/1.1");
    // send the HTTP PUT request:
    client.print("GET /api/post?json={bike-");
    client.print(bikeID);
    client.print(":");
    client.print(period);
    client.println("} HTTP/1.1");
    client.println("Host: ");
    client.println("User-Agent: Arduino-ethernet");
    client.println("Connection: close");
    client.println();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("Connection failed");
    Serial.println("Disconnecting.");
    client.stop();
    receiving = false;
  }
}

