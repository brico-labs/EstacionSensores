// Subido el 16/7/2013
// aparentemente funciona bien en la subida de datos.
// Los datos se pueden ver en https://xively.com/feeds/124735/


// Lo que no funciona
// Actualmente el comando xivelyclient.get da resultados erroneos y no es detectado por Xilevy

// Lo que falta o se puede añadir
// Nuevos sensores
// recoger un lod de los datos en local en tarjeta SD
// Filtrar los datos
// Añadir una pantalla en local para ver que pasa
// Otras ideas

/**
* Install next needed libraries into your libraries directory
* Library HttpClient: https://github.com/amcewen/HttpClient/archive/master.zip or $ git clone git://github.com/amcewen/HttpClient.git HttpClient
* Library Xively: https://github.com/xively/xively_arduino/archive/master.zip or $ git clone git://github.com/xively/xively-arduino.git XivelyArduino
* Library Adafruit_BMP085: https://github.com/adafruit/Adafruit-BMP085-Library
* Library WiFi Shield: https://github.com/arduino/wifishield/archive/master.zip
*/

// Link to Bricolabs feed https://xively.com/feeds/124735


#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <Xively.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

char ssid[] = "bricolabs"; // network SSID (name)
                           // In an open network password and keyIndex are  not neccesary 
// char pass[] = ""; // network password (use for WPA, or use as key for WEP)
                     // WEP password must be in HEX. Its necessary to convert 13 leng ASCII to HEX
                     // trere is a conversor at: http://www.seguridadwireless.net/php/conversor-universal-wireless.php
// int keyIndex = 0; // network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Xively key to let you upload data
#define xivelyKey "seiuHmN24Nwtq8iOuVIUXGuF2oOSAKxEZHd5UzNZcjJOZz0g" // Bricolabs Xively/Cosm API key
#define FEED_ID 124735 // Bricolabs Xively/Cosm feed ID

const short LUM_PIN = 0;
const short TEMP_PIN = 1;

int putes = 0;
int puterrs = 0;
int getes = 0;
int geterrs = 0;

// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;
// Define the strings for our datastream IDs
char Luminosidad[] = "Luminosity";
char Temperatura[] = "Temperature";
char Pressure[] = "Pressure";
char Altitud[] = "Altitude";
XivelyDatastream datastreams[] = {
  XivelyDatastream(Luminosidad, strlen(Luminosidad), DATASTREAM_FLOAT),
  XivelyDatastream(Temperatura, strlen(Temperatura), DATASTREAM_FLOAT),
  XivelyDatastream(Pressure, strlen(Pressure), DATASTREAM_FLOAT),
  XivelyDatastream(Altitud, strlen(Altitud), DATASTREAM_FLOAT),  
};

// wrap the datastreams into a feed
XivelyFeed feed(FEED_ID, datastreams, 4 /* number of datastreams */);

WiFiClient client;
XivelyClient xivelyclient(client);

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("Starting multiple datastream upload to Xively...");
  Serial.println();
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid);                  // for an open WiFi network
    // status = WiFi.begin(ssid, keyIndex, pass); // for an closed WiFi network
    // wait 10 seconds for connection:
    delay(5000);
    if( status != WL_CONNECTED)
      delay(5000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  bmp.begin();
}

void loop() {

  //int sensorValue = analogRead(sensorPin);
  float lumValue = map(analogRead(LUM_PIN) ,0,1023,0,100);
  float tempValue =bmp.readTemperature();
  float pressValue = bmp.readPressure()/100.0;
  float altValue = bmp.readAltitude();

  datastreams[0].setFloat(lumValue);
  datastreams[1].setFloat(tempValue);
  datastreams[2].setFloat(pressValue);
  datastreams[3].setFloat(altValue);

  Serial.print(millis());

  Serial.print(" Read sensor value ");
  Serial.print(datastreams[0].getFloat());

  Serial.print(" >> ");
  Serial.print(datastreams[1].getFloat());

  Serial.print(" >> ");
  Serial.println(datastreams[2].getFloat());
  
  Serial.print(" >> ");
  Serial.println(datastreams[3].getFloat());
  
  Serial.print("feed");
  Serial.println(feed);

  Serial.print("Uploading it to Xively. ");
  int ret = xivelyclient.put(feed, xivelyKey);
  putes ++;
  if (ret < 0) {
    puterrs ++;
  }
  Serial.print("xivelyclient.put returned ");
  Serial.print(ret);
  Serial.print(" errores = ");
  Serial.print(puterrs);
  Serial.print(" / ");
  Serial.println(putes);

  Serial.print ("tempValue = ");
  Serial.println ((5.0 * analogRead(TEMP_PIN) * 100.0)/1024.0);
  delay(1000);

  Serial.print ("tempValue = ");
  Serial.println ((5.0 * analogRead(TEMP_PIN) * 100.0)/1024.0);
  delay(1000);

  Serial.print ("tempValue = ");
  Serial.println ((5.0 * analogRead(TEMP_PIN) * 100.0)/1024.0);
  delay(1000);


  // -----------------
  Serial.println();
  Serial.println();
  Serial.print("feed antes de get > ");
  Serial.print(feed);
  ret = xivelyclient.get(feed, xivelyKey);

  Serial.print("feed despues de get > ");
  Serial.print(feed);

  Serial.print(millis());
  Serial.print(" xivelyclient.get returned ");
  Serial.print(ret);
  getes ++;
  if (ret < 0) {
    geterrs ++;
  }
  Serial.print(" errores = ");
  Serial.print(geterrs);
  Serial.print(" / ");
  Serial.println(getes);


  if (ret > 0)
  {
    Serial.print("Datastream is... ");
    Serial.println(feed[0]);
    Serial.print("Temperature is: ");
    Serial.println(feed[1]);
    Serial.print("Pressure is: ");
    Serial.println(feed[2]);
    Serial.print("Altitude aprox. is: ");
    Serial.println(feed[3]);
    
    Serial.print(feed[0].getFloat());
    Serial.print(" >> ");
    Serial.print(feed[1].getFloat());
    Serial.print(" >> ");
    Serial.print(feed[2].getFloat());
    Serial.print(" >> ");
    Serial.print(feed[3].getFloat());
    Serial.println();

  }
  Serial.println();
  delay(15000UL);
}
