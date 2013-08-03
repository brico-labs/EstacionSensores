// Subido el 16/7/2013
// aparentemente funciona bien en la subida de datos.
// Los datos se pueden ver en https://xively.com/feeds/124735/


// Lo que no funciona
// Actualmente el comando xivelyclient.get da resultados erroneos y no es detectado por Xilevy

// Lo que falta o se puede añadir
// Nuevos sensores
// recoger un log de los datos en local en tarjeta SD
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
#include <LiquidCrystal.h>

//******  DEFINE CONSTANTES  ******

// Xively key to let you upload data
#define xivelyKey "seiuHmN24Nwtq8iOuVIUXGuF2oOSAKxEZHd5UzNZcjJOZz0g" // Bricolabs Xively/Cosm API key
#define FEED_ID 124735 // Bricolabs Xively/Cosm feed ID

const short LUM_PIN = 0;


//******  DEFINE VARIABLES  ******
 char ssid[] = "bricolabs"; // network SSID (name)
                           // In an open network password and keyIndex are  not neccesary 
// char pass[] = ""; // network password (use for WPA, or use as key for WEP)
                     // WEP password must be in HEX. Its necessary to convert 13 leng ASCII to HEX
                     // trere is a conversor at: http://www.seguridadwireless.net/php/conversor-universal-wireless.php
// int keyIndex = 0; // network key Index number (needed only for WEP)




int status = WL_IDLE_STATUS;
// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;
// Define the strings for our datastream IDs
unsigned long time, time2, intervalo;
int medida;

char* canales[4] = {"Luminosity", "Temperature", "Pressure", "Altitude"};
char* shortCanales[4] = {"Lumin", "Temp", "Pres", "Altit"};
char* unidades[4] = {" %", " -C", " hPa", " m"};


XivelyDatastream datastreams[] = {
  XivelyDatastream(canales[0], strlen(canales[0]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[1], strlen(canales[1]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[2], strlen(canales[2]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[3], strlen(canales[3]), DATASTREAM_FLOAT),  
};

// wrap the datastreams into a feed
XivelyFeed feed(FEED_ID, datastreams, 4 /* number of datastreams */);




//******  CREA INSTANCIAS DE CLASES  ******

Adafruit_BMP085 bmp;
LiquidCrystal lcd(9, 8, 6, 5, 3, 2);
WiFiClient client;
XivelyClient xivelyclient(client);



void setup() 
{
  Serial.begin(9600);
  bmp.begin();
  lcd.begin(16,2);

  time = time2 = millis();
  intervalo = 60; // Intervalo entre dos lecturas y envio de datos en segundos

      Serial.println("Starting multiple datastream upload to Xively...");
      Serial.println();
      
      
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
      lcd.clear(); lcd.print("Conectando con "); lcd.setCursor(2, 1); lcd.print(ssid);
    
    status = WiFi.begin(ssid);                  // for an open WiFi network
    //status = WiFi.begin(ssid, keyIndex, pass); // for an closed WiFi network
    // wait 10 seconds for connection:
    delay(10000);
    if( status != WL_CONNECTED)
      delay(5000);
    }

  printWifiStatus();

  Serial.println("Connected to wifi");
  Serial.println("---------------");

    lcd.clear(); lcd.print("Conectada con"); lcd.setCursor(2, 1); lcd.print(ssid);
        delay (5000);
    medida = 1;
}



void loop() {
  int ret;
  
  if (millis() > time + intervalo * 1000)
  {
    medida = 1;
  }
  
  if (medida != 0)
  {
    medida = 0;
    time = millis();
  
    float lumValue   = map(analogRead(LUM_PIN) ,0,1023,0,100);
    float tempValue  = bmp.readTemperature();
    float pressValue = bmp.readPressure() / 100.0;
    float altValue   = bmp.readAltitude();

    datastreams[0].setFloat(lumValue);
    datastreams[1].setFloat(tempValue);
    datastreams[2].setFloat(pressValue);
    datastreams[3].setFloat(altValue);

    printValues();

    Serial.print("          Uploading it to Xively...  ");

    lcd.clear();
    lcd.print("PUT to Xively");

    ret = xivelyclient.put(feed, xivelyKey);

    Serial.print(" returned ");
    Serial.println(ret);
    Serial.println();
  
    lcd.print(" ... ");
    if (ret == 200) lcd.print("Ok");
    else lcd.print(ret);
    delay(1000);
    
  }
  
  else if ((millis() - time)/1000 == intervalo/2)
  {

    // -----------------
    Serial.print("     Getting data from Xively...  ");

    lcd.clear();
    lcd.print("GET from Xively");

    ret = xivelyclient.get(feed, xivelyKey);
    Serial.print(" returned ");
    Serial.println(ret);
  
    lcd.setCursor(4, 1);
    lcd.print("return ");
    if (ret == 200) lcd.print("Ok");
    else lcd.print(ret);

    if (ret = 200)
    {
      printValues();
    }
    else Serial.println();
    
    Serial.println();
        delay(1000);

  }
  
  else if (millis() > time2 + 1000 )
  {
    int pendiente = intervalo - intervalo * (time2-time)/(intervalo * 1000);
    time2 = millis();
    
    int i = (pendiente / 5) % 4;    
    lcd.clear();

    lcd.setCursor(0, 1);
    lcd.print("faltan "); lcd.print(pendiente); lcd.print(" seg"); 
    
    lcd.setCursor(0, 0);
    lcd.print(shortCanales[i]); lcd.print (": "); lcd.print(datastreams[i].getFloat()); lcd.print(unidades[i]);

/*    
    switch (i)
    {
      case 0:
      lcd.print("Lumi: "); lcd.print(datastreams[i].getFloat()); lcd.print(" %"); break;
      case 1: 
      lcd.print("Temp: "); lcd.print(datastreams[i].getFloat()); lcd.print(" ºC"); break;
      case 2:
      lcd.print("Pres: "); lcd.print(datastreams[i].getFloat()); lcd.print(" hPa"); break;
      case 3:
      lcd.print("Alt: "); lcd.print(datastreams[i].getFloat()); lcd.print(" m"); break;
    }

*/    
  }
 
}






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

void printValues(){        
  Serial.print(">> Sensor values ");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(shortCanales[i]);
    Serial.print(" = ");
    Serial.print(datastreams[i].getFloat());
    Serial.print(unidades[i]);
    Serial.print("  ");
  }
  Serial.println();
}
  
