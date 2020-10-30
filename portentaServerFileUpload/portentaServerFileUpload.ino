/*
  Portenta Server File Upload
  A simple web server that lets you send files attached as post request via the web.

  This sketch will print the IP address of your WiFi module (once connected)
  to the Serial monitor. From there, you can open that address in a web browser

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the Wifi.begin() call accordingly.

  This file is initially for testing Portenta H7 WiFi
  increase timeout if data is clipped
  edit WiFiClient.cpp
  in arduino::WiFiClient::receiveData()
  //_socket->set_blocking(false);
  _socket->set_timeout(20); or higher

  created 30 Okt 2020
  by Andreas Donner
*/

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
    // wait 10 seconds for connection:
    delay(10000);
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
    uint8_t i = 0;                          // empty lines counter
    bool eof = false;                       // end of file
    long b = 0;                             // byte counter
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
            if (i >= 2) {   // data section
              Serial.println("data:");
              break;
            }
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }

      // second loop for the data part
      char c[7];                              // buffer
      char eofString[] = "------W";           // eof string

      while (client.available()) {

        for (uint8_t z = 0; z < 6; z++) {     // shift
          c[z] = c[z + 1];
        }
        c[6] = client.read();                 // read a byte, then

        if (strstr(c, eofString)) {           // check for eof
          eof = true;
          b = b - 8;
          break;
        } else {
          b++;                                // increase byte counter
          if (b > 7) {
            Serial.write(c[0]);               // print it out the serial monitor
            // TODO
            // save data to SD or do something useful
          }
        }
      }

      // if it is a post request, there are bytes behind the two newline characters
      if (i >= 1) {

        // send a response, add the current post parameter line:
        if (eof) {
          char retStr[60];
          sprintf(retStr, "transfer completed<br> %d bytes received", b);
          sendFileDiag(client, retStr);
        }
        else sendFileDiag(client, currentLine);

        // break out of the while client.connected loop:
        break;
      }
      // increase if the two newline characters are missing
      i++;
    }

    // close the connection:
    client.stop();
    Serial.println();
    Serial.println("client disconnected");
  }
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

void sendFileDiag(WiFiClient & client, String currentLine)
{
  IPAddress ip = WiFi.localIP();
  client.print(F("HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "\r\n"
                 "<!doctype html>\n"
                 "<html lang='en'>\n"
                 "<head>\n"
                 "<meta charset='utf-8'>\n"
                 "<meta name='viewport' content='width=device-width'>\n"
                 "<title>Portenta H7 File Upload</title>\n"
                 "</head>\n"
                 "<body style='font-family:Helvetica, sans-serif'>\n"
                 "<h1>Portenta H7 File Upload</h1>\n"
                 "<form enctype='multipart/form-data' action='http://"));
  client.print(ip);
  client.print(F("' method='POST'>\n"));
  //client.print(F("<input type='hidden' name='MAX_FILE_SIZE' value='100000' />\n"));
  client.print (F("<p>Choose a file to upload:</p><input name='uploadedfile' type='file' /><br />\n"));
  client.print (F("<input type='submit' value='Upload File' />\n"));
  client.print (F("</form>\n"));
  client.print(F( "<br>\n<p>"));
  client.print (currentLine);
  client.print (F("</p></body>\n</html>"));
}
