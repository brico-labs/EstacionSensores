// Version 2014-11-04


// Configuracion de la aplicacion
unsigned long intervalo = 60 ;   // Intervalo entre dos intentos de subida a Xively
boolean  imprime = true;        // Imprime los valores actualizados
float alfa = 0.9;
int numero_valores = 10;


// Lo que falta o se puede añadir
// Nuevos sensores
// recoger un log de los datos en local en tarjeta SD
// Filtrar los datos

/**
* Install next needed libraries into your libraries directory
* Library HttpClient: https://github.com/amcewen/HttpClient/archive/master.zip or $ git clone git://github.com/amcewen/HttpClient.git HttpClient
* Library Xively: https://github.com/xively/xively_arduino/archive/master.zip or $ git clone git://github.com/xively/xively-arduino.git XivelyArduino
* Library Adafruit_BMP085: https://github.com/adafruit/Adafruit-BMP085-Library
* Library WiFi Shield: https://github.com/arduino/wifishield/archive/master.zip

Incluye algunas funciones matematicas incluidas en http://www.nongnu.org/avr-libc/user-manual/group__avr__math.html
*/

// Link to Bricolabs feed https://xively.com/feeds/124735

#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <Xively.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>




// Configuracion de terminales usados como entradas y salidas
// Medidor de luminosidad
    const short LUM_PIN =  A2;     // Entrada analogica para el sensor de luz

// Patas de control para el sensor de polvo SHARP GP2Y1010AU0F
    const int DUST_PIN_I = A4;     // Entrada digital del sensor de polvo
    const int DUST_PIN_O = 53;     // Salida digital del sensor de polvo
    float V_sin_polvo =    0.38;    // Tensi´n medida sin polvo. Se corrige en funcion de las medidas reales
    float ganancia_polvo = 5.0;    // ganancia en mg/m3 por cada V.
    const int NUMERO_MED_PSUSP = 20;



// Configuracion del sensor de temperatura y humedad DHT11
#include <DHT.h>
#define DHTPIN 30              // Pin digital en el que se conecta el sensor DHT11
#define DHTTYPE DHT11          // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);      // Crea una instancia tipo dht para manejar el sensor
float temperatDHR;
boolean error_DHT;



// Configuracion del sensor de presion y temperatura BMP085
Adafruit_BMP085 bmp;    // Crea una instancia tipo BMP085


/*******************Demo for MG-811 Gas Sensor Module V1.1*****************************
Original
Author:  Tiequan Shao: tiequan.shao@sandboxelectronics.com
         Peng Wei:     peng.wei@sandboxelectronics.com
         
Lisence: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)

Note:    This piece of source code is supposed to be used as a demostration ONLY. More
         sophisticated calibration is required for industrial field application. 
         
                                                    Sandbox Electronics    2012-05-31
Configuracion del sensor de CO2  MG-811
Comprado en http://sandboxelectronics.com/?product=mg-811-co2-gas-sensor-module

/*********************** Hardware Related Macros ***********************************/
#define MG_PIN    (1)     //define which analog input channel you are going to use
#define BOOL_PIN (28)
/***************************** Calibracion ****************************************
Calibrado de acuerdo con http://www.veetech.org.uk/Prototype_CO2_Monitor.htm
These two values differ from sensor to sensor. user should derermine this value.

ZERO_POINT_VOLTAGE   es el voltaje de medida del sensor en aire limpio (400ppm)
TOTAL_POINT_VOLTAGE  es el voltaje medido con una concetracion de 40.000 ppm)

A mayores incluye un ajuste dinamico del punto de aire limpio:
       En caso de que el valor de tension medida sea mayor que el que corresponde a 
       aire limpio (400ppm)toma ese valor de tension como valor para aire limpio
       Valores medidos: */
       float ZERO_POINT_VOLTAGE  = 0.400;
       float TOTAL_POINT_VOLTAGE = 0.315;
       float REACTION_VOLTGAE    = ZERO_POINT_VOLTAGE - TOTAL_POINT_VOLTAGE;
/***********************************************************************************/






// Configuracion del sensor de MQ-2 (CO y otros gases)

/*******************Demo for MQ-2 Gas Sensor Module V1.0*****************************
Support:  Tiequan Shao: support[at]sandboxelectronics.com
 
Lisence: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 
Note:    This piece of source code is supposed to be used as a demostration ONLY. More
         sophisticated calibration is required for industrial field application. 
 
                                                    Sandbox Electronics    2011-04-25
************************************************************************************/
 
/************************Hardware Related Macros************************************/
#define         MQ_PIN                       (3)     //define which analog input channel you are going to use
 
/***********************Software Related Macros************************************/
int CALIBARAION_SAMPLE_TIMES    =  50;    //define how many samples you are going to take in the calibration phase
int CALIBRATION_SAMPLE_INTERVAL = 100;    //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (2)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
                                                     
/*****************************Globals***********************************************/
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
float           rs = 0;
int   v_media = 0;
String textos_err_CO2;

String textos_medidas [10];

// Configuracion del Display
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


// Configuracion de Xively: Xively key to let you upload data
#define xivelyKey "seiuHmN24Nwtq8iOuVIUXGuF2oOSAKxEZHd5UzNZcjJOZz0g" // Bricolabs Xively/Cosm API key
#define FEED_ID 124735 // Bricolabs Xively/Cosm feed ID


char* canales[9]      = {"Luminosidad", "Temperatura", "Presion", "CO2", "V_CO2x1000", "CO", "DHT_HR", "Part_Susp", "Notas"};
char* shortCanales[9] = {"Lumin", "Temp ", "Pres ", "CO2  ", "V_CO2", "CO   ", "HR   ", "Psusp", "Notas"};
char* unidades[9]     = {" s/u", " -C ", " hPa", " ppm", "V", " ppm", " %  ", " mg/m3", " __"};

const int bufferSize = 140;
char bufferValue[bufferSize]; // enough space to store the string we're going to send

String stringId("random_string");

XivelyDatastream datastreams[] = {
  XivelyDatastream(canales[0], strlen(canales[0]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[1], strlen(canales[1]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[2], strlen(canales[2]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[3], strlen(canales[3]), DATASTREAM_FLOAT),  
  XivelyDatastream(canales[4], strlen(canales[4]), DATASTREAM_FLOAT),  
  XivelyDatastream(canales[5], strlen(canales[5]), DATASTREAM_FLOAT),  
  XivelyDatastream(canales[6], strlen(canales[6]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[7], strlen(canales[7]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[8], strlen(canales[8]), DATASTREAM_BUFFER, bufferValue, bufferSize),
  XivelyDatastream(stringId, DATASTREAM_STRING)
};

float   valor_medido   [10], valor_filtrado  [10];
boolean primera_medida [10], actualiza_medida [10], actualiza_medidas = true;

// wrap the datastreams into a feed
XivelyFeed feed(FEED_ID, datastreams, 10 /* number of datastreams */);

WiFiClient client;
XivelyClient xivelyclient(client);


/*
// Configuracion de la conexion Wifi
char ssid[] = "Makers"; // network SSID (name)
                           // In an open network password and keyIndex are not neccesary 
char pass[] = "OSHWDem2014Domus";          // network password (use for WPA, or use as key for WEP)
                           // WEP password must be in HEX. It's necessary convert 13 leng ASCII to HEX
                           // there is a conversor at: http://www.seguridadwireless.net/php/conversor-universal-wireless.php
//int keyIndex = 0;        // network key Index number (needed only for WEP)
*/

/*
// Configuracion de la conexion Wifi
char ssid[] = "Laboratorio Domus"; // network SSID (name)
                           // In an open network password and keyIndex are not neccesary 
char pass[] = "s1ncables";          // network password (use for WPA, or use as key for WEP)
                           // WEP password must be in HEX. It's necessary convert 13 leng ASCII to HEX
                           // there is a conversor at: http://www.seguridadwireless.net/php/conversor-universal-wireless.php
//int keyIndex = 0;        // network key Index number (needed only for WEP)
*/


char ssid[] = "sesta"; // network SSID (name)
                           // In an open network password and keyIndex are not neccesary 
char pass[] = "31333935353030303030303030";          // network password (use for WPA, or use as key for WEP)
                           // WEP password must be in HEX. It's necessary convert 13 leng ASCII to HEX
                           // there is a conversor at: http://www.seguridadwireless.net/php/conversor-universal-wireless.php
int keyIndex = 1;          // network key Index number (needed only for WEP)


int status = WL_IDLE_STATUS;


// Variables auxiliares
unsigned long time, time2, segundos, secuencia;
int medida;
int ret;                        // se usa como control de respuesta en algunas llamadas a funciones
String text_result_1 = "   --     ";

String cuarta_linea = "--------------------";

// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;
boolean pulso = LOW;
int ciclos = 0;
  
float humedadDHR;
float CO2Value, volts_CO2;
float lumValue;

float tempValue;
float pressValue;
float altValue;

float concentracion_CO;
  
float concentracion_polvo;
float desviacion_polvo;
  



void setup() 
{
  Serial.begin(9600);        // Inicia protocolo de conexion serie

//  lcd.begin(16,2);
  lcd.begin(20,4);           // Inicia conexion con LCD

  bmp.begin();               // Inicia conexi´n con sensor bmp-811
  dht.begin();               // inicializa el sensor DHT
  
  pinMode (9, OUTPUT);       // este LED parpadea al ritmo de cada cicko de medidas
  
      Serial.println("Starting multiple datastream upload to Xively...");
      Serial.println();
 
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
      lcd.clear();  lcd.print("Conectando con ");  lcd.setCursor(2, 1);  lcd.print(ssid);
    
    // status = WiFi.begin(ssid);                        // for an open WiFi network
    //status = WiFi.begin(ssid, pass);                // for an closed WPA WiFi network
    status = WiFi.begin(ssid, keyIndex, pass);      // for an closed WEP WiFi network
    // wait 10 seconds for connection:
    delay(10000);
    if( status != WL_CONNECTED) delay(5000);
  }

  printWifiStatus();

  Serial.println("Connected to wifi");
  Serial.println("---------------");

    lcd.setCursor(9,2);
    lcd.print("Conectada");
    medida = 1;
    
  // Calibracion del sensor MQ-2
  Serial.print("Calibrating...\n");                
    lcd.setCursor(0,4);
    lcd.print("Calibrando CO");
    Ro = MQCalibration(MQ_PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");
 
  for (int i = 0; i < numero_valores; i ++)
  {
    primera_medida   [i] = true;
    actualiza_medida [i] = true;
  }

  
  time2 = millis () + 2000;   // Inicia el intervalo de subida de datos
  time = millis ();           // Inicia el intervalo de subida de datos
  
  

}

    


void loop()
{
  if (actualiza_medidas == true)
  {
    // Señaliza con el LED 9 que se actualiz´ una medida
    digitalWrite (9, HIGH); delay (50); digitalWrite (9, LOW);
    ciclos = ciclos ++;  ciclos = ciclos % 6;  

    switch (ciclos)
    {
      case 0:
        // Lee el medidor de luminosidad LDR
        lumValue   = map(analogRead(LUM_PIN) ,0,1023,0,100);
          valor_medido  [0] =lumValue;
      break;
      
      case 1:
        // Lee los valores de presion y temperatura del BMP
        tempValue = bmp.readTemperature();
          valor_medido  [1] = tempValue;
        pressValue = bmp.readPressure() / 100.0;
          valor_medido  [2] = pressValue;
        //altValue   = bmp.readAltitude();
      break;

      case 2:
        // Lee los valores del sensor de CO2
        CO2Value = MGRead(MG_PIN);
          valor_medido  [3] = CO2Value;
          valor_medido  [4] = volts_CO2 * 1000.0;
      break;

      case 3:
        // Lee el medidor de CO
        concentracion_CO = MQRead(MQ_PIN);
          valor_medido  [5] = concentracion_CO;
          textos_medidas [5] = textos_err_CO2;
          
          Serial.print (textos_err_CO2);
          Serial.print ("  -->  ");
          Serial.println (textos_medidas [5]);
      break;

      case 4:
        // Lee los valores del sensor DHT
        leeSensorDHT();        
          valor_medido [6] = humedadDHR;
      break;

      case 5:
        // Lee la concentracion de polvo
        concentracion_polvo = concentracionPolvo ();
          valor_medido  [7] = concentracion_polvo;
          valor_medido  [8] = desviacion_polvo;
      break;
    }
  }


  actualiza_medidas = false;
  for (int i = 0; i < numero_valores; i ++)
  {
    if (valor_medido [i] > -100)       // medida correcta
    {
      if (primera_medida [i] = true)
      {
        valor_filtrado   [i] = valor_medido [i];
        primera_medida   [i] = false;
        actualiza_medida [i] = false;
      }
      else
      {
        valor_filtrado   [i] = valor_filtrado [i] * (1 - alfa);
          valor_filtrado [i] = valor_filtrado [i] + valor_medido [i] * alfa;
        actualiza_medida [i] = false;
      }
    }
    else
    {
      actualiza_medida [i] = true;
      actualiza_medidas    = true;

    }
  }



 
 
  if (millis() > time + intervalo * 1000)  // Sube datos cada "intervalo" segundos.
  { 
    for (int i = 0; i < 8; i++)
    {
      datastreams[i].setFloat (valor_filtrado [i]);
    }
    datastreams[8].setBuffer ("sin novedad");  

    printValues();
    Serial.print("          Uploading it to Xively...  ");
      lcd.clear();
      lcd.print("PUT to Xively");

    ret = xivelyclient.put(feed, xivelyKey);

    Serial.print(" returned ");
    Serial.print(ret);
    if (ret == 200) Serial.println("  Ok ");
    Serial.println();
    Serial.println();

    lcd.print(" ... ");
    if (ret == 200) 
    {
      lcd.print("Ok");
      text_result_1 = "   Ok";
    }
    else 
    {
      lcd.print(ret);
      text_result_1 = "Err " + String (ret);
    }    
    time = millis();
    
    segundos = millis() / 1000 + 5;
    secuencia = -1;                                         // Fuerza que la primera linea que se actualice en el Display sea la primera
  }
  
  
  if (millis() / 1000 > segundos)
  {
    String valor_txt;
    String unidad;
    String texto;

    segundos = millis() / 1000;
    secuencia ++;
    
    int linea         = secuencia % 3;                        // linea del display en la que se va a enseñar el texto
    int indice_medida = secuencia % 7;                        // medida a mostrar
                                                             // Forma el texto para mostrar
    String parametro = shortCanales[indice_medida] + String (": "); // Nombre del parametro

    float  valor     = valor_filtrado [indice_medida];       // Valor de la variable con formato xxxx.xx    


          Serial.print ("\t\t  -->  ");
          Serial.print (textos_err_CO2);
          Serial.print ("  -->  ");
          Serial.println (textos_medidas [5]);


    if (valor_medido [indice_medida] < -99)
    {
          Serial.print ("\t\t  -->  ");
          Serial.print (textos_err_CO2);
          Serial.print ("  -->  ");
          Serial.print (textos_medidas [indice_medida]);
          Serial.print ("  -->  ");
          Serial.println (indice_medida);


      texto = parametro + textos_medidas [indice_medida];
      Serial.print ("     el texto correspondiente a medida ");
      Serial.print (indice_medida);
      Serial.print (" es ***");
      Serial.print (texto);
      Serial.print ("***");
      Serial.print ("\t valor de la medida ");
      Serial.print (valor_medido [indice_medida]);
      Serial.print ("\t texto origen ");
      Serial.print (textos_medidas [indice_medida]);
      Serial.print ("\t CST = ");
      Serial.print (CALIBARAION_SAMPLE_TIMES);
      Serial.println ();
      
    }
    else
    {
      unidad    = String (unidades[indice_medida]);             // Unidad de medida
      texto = parametro + float_to_text (valor, 2) + unidad;
    }

    if (texto.length() < 20)
    {
      for (int i = texto.length(); i < 20; i++) 
      { texto += " ";}
    }
    else if (texto.length() > 20)
    {
      String auxiliar = texto;
      texto = "";
      for (int i = 0; i < 20; i++) 
      {
        texto [i] = auxiliar [i];
      }
    }
      
    for (int i = 0; i < numero_valores; i ++)
    {
      actualiza_medida [i] = true;
    }
  
      


    lcd.setCursor (0, linea);                                // Presenta el texto
    lcd.print (texto);  
    
    int  tiempo_restante = int (intervalo - (millis() - time) / 1000);
    //unsigned long  tiempo_restante = int (intervalo - (millis() - time) / 1000);
        
    lcd.setCursor (0, 3);                                    // Info sobre la ultima subida
    lcd.print (String (" ") + text_result_1);

    String auxiliar1 = text_result_1 + "          ";
    String auxiliar2 = String ("Prox ") + String ( tiempo_restante) + String ("s           ");

    cuarta_linea;
    for  (int i = 0; i < 10; i ++)
    {
      cuarta_linea [i]      = auxiliar1 [i];                // Info sobre la ultima subida
      cuarta_linea [i + 10] = auxiliar2 [i];                // tiempo restante
    }

    lcd.setCursor (0, 3);   
    lcd.print (cuarta_linea);
  }
  
  actualiza_medidas    = true;
}


