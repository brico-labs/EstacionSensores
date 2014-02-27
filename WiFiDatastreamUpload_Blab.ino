// Lo que falta o se puede añadir
// Nuevos sensores
// recoger un log de los datos en local en tarjeta SD
// Filtrar los datos
// Otras ideas

/**
* Install next needed libraries into your libraries directory
* Library HttpClient: https://github.com/amcewen/HttpClient/archive/master.zip or $ git clone git://github.com/amcewen/HttpClient.git HttpClient
* Library Xively: https://github.com/xively/xively_arduino/archive/master.zip or $ git clone git://github.com/xively/xively-arduino.git XivelyArduino
* Library Adafruit_BMP085: https://github.com/adafruit/Adafruit-BMP085-Library
* Library WiFi Shield: https://github.com/arduino/wifishield/archive/master.zip
*/

// Link to Bricolabs feed https://xively.com/feeds/124735




/*******************Demo for MG-811 Gas Sensor Module V1.1*****************************
Author:  Tiequan Shao: tiequan.shao@sandboxelectronics.com
         Peng Wei:     peng.wei@sandboxelectronics.com
         
Lisence: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)

Note:    This piece of source code is supposed to be used as a demostration ONLY. More
         sophisticated calibration is required for industrial field application. 
         
                                                    Sandbox Electronics    2012-05-31
************************************************************************************/

/************************Hardware Related Macros************************************/
#define         MG_PIN                       (1)     //define which analog input channel you are going to use
//#define         BOOL_PIN                     (2)
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier


/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in 
                                                     //normal operation

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
//#define         ZERO_POINT_VOLTAGE           (0.475) //define the output of the sensor in volts when the concentration of CO2 is 400PPM. Ahora 0.540
#define         ZERO_POINT_VOLTAGE           (0.565)
#define         REACTION_VOLTGAE             (0.028) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2. Ahora 0.020

/*****************************Globals***********************************************/
float           CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))};   
                                                     //two points are taken from the curve. 
                                                     //with these two points, a line is formed which is
                                                     //"approximately equivalent" to the original curve.
                                                     //data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280) 
                                                     //slope = ( reaction voltage ) / (log400 ¨Clog1000) 

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

const short LUM_PIN = 10;

const CO_PIN =    A11;     // Entrada analogica conectada al sensor de CO

const DTH11_PIN =  21;     // Entrada digital de conexión con DHT11

                           // Patas de control para el sensor de polvo
                           // SHARP GP2Y1010AU0F
const DUST_PIN_O =  20;    // Salida digital del sensor de polvo
const DUST_PIN_I = A13;    // Entrada digital del sensor de polvo


//******  DEFINE VARIABLES  ******
char ssid[] = "bricolabs"; // network SSID (name)
                           // In an open network password and keyIndex are not neccesary 
char pass[] = "s1ncables";          // network password (use for WPA, or use as key for WEP)
                           // WEP password must be in HEX. It's necessary convert 13 leng ASCII to HEX
                           // there is a conversor at: http://www.seguridadwireless.net/php/conversor-universal-wireless.php
//int keyIndex = 0;        // network key Index number (needed only for WEP)


int status = WL_IDLE_STATUS;
// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;
// Define the strings for our datastream IDs
unsigned long time, time2, intervalo;
int medida;

char* canales[4] = {"Luminosity", "Temperature", "Pressure", "CO2"};
char* shortCanales[4] = {"Lumin", "Temp", "Pres", "CO2"};
char* unidades[4] = {" %", " -C", " hPa", " ppm"};


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

// LiquidCrystal lcd(9, 8, 6, 5, 3, 2);      //Esta es la llamada antigua a la instancia LCD

                                          //Esta es la llamada con las patas reordenadas 
LiquidCrystal lcd(3, 5, 10, 11, 12, 13);  //para el montaje en el Mega (24/01/14)

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
    
    status = WiFi.begin(ssid);                        // for an open WiFi network
    //status = WiFi.begin(ssid, pass);                // for an closed WPA WiFi network
    //status = WiFi.begin(ssid, keyIndex, pass);      // for an closed WEP WiFi network
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
    
       Serial.print("MG-811 Demostration\n");                

}

void loop() {
  int ret;
  long percentage;
  float volts;

  if (millis() > time + intervalo * 1000)
  {
    medida = 1;
  }
  
  if (medida != 0)
  {
    medida = 0;
    time = millis(); 
   
    volts = MGRead(MG_PIN);
    percentage = MGGetPercentage(volts,CO2Curve);
    
    Serial.print( "SEN-00007: " );
    Serial.print(volts); 
    Serial.print( "V           " );
    Serial.print("CO2: ");
    if (percentage == -1) {
        Serial.print( "<400" );
    } else {
        Serial.print(percentage);
    } 
    Serial.print( " ppm" );  
    Serial.print("\n");


  
    float lumValue   = map(analogRead(LUM_PIN) ,0,1023,0,100);
    float tempValue  = bmp.readTemperature();
    float pressValue = bmp.readPressure() / 100.0;
    float altValue   = bmp.readAltitude();
    float CO2Value   = percentage;

    datastreams[0].setFloat(lumValue);
    datastreams[1].setFloat(tempValue);
    datastreams[2].setFloat(pressValue);
    datastreams[3].setFloat(CO2Value);

    printValues();

    Serial.print("          Uploading it to Xively...  ");

    lcd.clear();
    lcd.print("PUT to Xively");

    ret = xivelyclient.put(feed, xivelyKey);

    Serial.print(" returned ");
    Serial.print(ret);
    if (ret == 200) Serial.println("  Ok ");
    Serial.println();

  
    lcd.print(" ... ");
    if (ret == 200) lcd.print("Ok");
    else lcd.print(ret);
    delay(1000);
    
  }
  
  else if ((millis() - time)/1000 == intervalo/2)
  {

    /*
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
    
    */

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
    
       volts = MGRead(MG_PIN);
    percentage = MGGetPercentage(volts,CO2Curve);
    
    Serial.print( "SEN-00007: " );
    Serial.print(volts * 1000); 
    Serial.print( "mV           " );
    Serial.print(volts * 1000 / 8.5); 
    Serial.print( "mV           " );
    Serial.print("CO2: ");
    if (percentage == -1) {
        Serial.print( "<400" );
    } else {
        Serial.print(percentage);
    } 
    Serial.print( " ppm" );  
    Serial.print("\n");
    
    float CO2Value   = percentage;

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
  

/*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN-000007
Remarks: This function reads the output of SEN-000007
************************************************************************************/ 
float MGRead(int mg_pin)
{
    int i;
    float v=0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;  
}

/*****************************  MQGetPercentage **********************************
Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(MG-811 output) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
long  MGGetPercentage(float volts, float *pcurve)
{
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else { 
       long ppm = pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
       Serial.print (ppm);
       Serial.print("  >>  ");
       return ppm;
   }
}
