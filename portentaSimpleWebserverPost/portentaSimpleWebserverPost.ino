#include "WiFi.h"
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;             // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);      // initialize serial communication
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  Serial.println("simple web server http post request");

  pinMode(LEDB, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 5 seconds for connection:
    delay(5000);
  }

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();

  // do this to avoid portenta h7 error blinking
  long rssi = WiFi.RSSI();

  //
  digitalWrite(LEDB, HIGH);
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
}

void loop() {

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    uint8_t i = 0;                          // counter
    String currentLine = "";                // make a String to hold incoming data from the client

    while (client.connected()) {            // loop while the client's connected

      Serial.println("client connected");   // print a message out the serial port

      while (client.available()) {          // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request:
          if (currentLine.length() == 0) {
            i++;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }

      // if it is a post request, there are bytes behind the two newline characters
      if (i >= 1) {
        if (currentLine.length() > 0) {
          char postParameter[currentLine.length() + 1];
          currentLine.toCharArray(postParameter, currentLine.length() + 1);

          if ( strncmp( postParameter, "ledC", 4) == 0 ) {
            byte pin = postParameter[4] - 48; // Convert ascii to int
            // ledR = 1, ledG = 2 ledB = 3
            if ( strncmp( postParameter + 5, "=On", 3) == 0 ) {
              if (pin == 1) digitalWrite(LEDR, LOW);
              else if (pin == 2) digitalWrite(LEDG, LOW);
              else if (pin == 3) digitalWrite(LEDB, LOW);
            }
            else if ( strncmp( postParameter + 5, "=Off", 4) == 0 ) {
              if (pin == 1) digitalWrite(LEDR, HIGH);
              else if (pin == 2) digitalWrite(LEDG, HIGH);
              else if (pin == 3) digitalWrite(LEDB, HIGH);
            }
          }

        }
        
        // send a response, add the current post parameter line:
        sendPage(client, currentLine);
        
        // break out of the while loop:
        break;
      }
      // increase if the two newline characters are missing
      i++;
    }

    // close the connection:
    client.stop();
    Serial.println();
    Serial.println("client disonnected");
  }
  
  /*
    digitalWrite(LEDR, LOW);
    delay(50);
    digitalWrite(LEDR, HIGH);
    delay(50);
  */
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void sendPage(WiFiClient &client, String currentLine)
{
  client.println(F("HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n"
                   "\r\n"
                   "<!doctype html>\n"
                   "<html lang='en'>\n"
                   "<head>\n"
                   "<meta charset='utf-8'>\n"
                   "<meta name='viewport' content='width=device-width'>\n"
                   "<title>Portenta H7 Webserver - simple post request test</title>\n"
                   "</head>\n"
                   "<body style='font-family:Helvetica, sans-serif'>\n"
                   "<h1>Portenta H7 Webserver - simple post request test</h1>\n"
                   "<p>LED Buttons</p>\n"
                   "<form method='post' action='/' name='ledswitch'>"));

  client.print(F("ledR is "));
  client.print(F(" <button type='submit' name='ledC"));
  client.print(1);
  if (!digitalRead(LEDR)) {
    client.print(F("' value='Off'>ON"));
  } else {
    client.print(F("' value='On'>OFF"));
  }
  client.print(F( "</button><br>\n"));

  client.print(F("ledG is "));
  client.print(F(" <button type='submit' name='ledC"));
  client.print(2);
  if (!digitalRead(LEDG)) {
    client.print(F("' value='Off'>ON"));
  } else {
    client.print(F("' value='On'>OFF"));
  }
  client.print(F( "</button><br>\n"));

  client.print(F("ledB is "));
  client.print(F(" <button type='submit' name='ledC"));
  client.print(3);
  if (!digitalRead(LEDB)) {
    client.print(F("' value='Off'>ON"));
  } else {
    client.print(F("' value='On'>OFF"));
  }
  client.print(F( "</button><br>\n"));

  client.print (F("</form>\n"));
  client.print(F( "<br>\n"));
  client.print(F( "previous post-parameter:\n"));
  client.print(F( "<br>\n<p>"));
  client.print (currentLine);

  client.print (F("</p></body>\n</html>"));
}
