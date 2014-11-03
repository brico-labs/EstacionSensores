// Version 2014-11-0


// Configuracion de la aplicacion
unsigned long intervalo = 60 ;   // Intervalo entre dos intentos de subida a Xively


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
#include <dht11.h>




// Configuracion de terminales usados como entradas y salidas
// Medidor de luminosidad
    const short LUM_PIN =  A2;     // Entrada analogica para el sensor de luz

// Patas de control para el sensor de polvo SHARP GP2Y1010AU0F
    const int DUST_PIN_I = A4;     // Entrada digital del sensor de polvo
    const int DUST_PIN_O = 53;     // Salida digital del sensor de polvo
    float V_sin_polvo =    0.38;    // Tensi´n medida sin polvo. Se corrige en funcion de las medidas reales
    float ganancia_polvo = 5.0;    // ganancia en mg/m3 por cada V.
    float concentracion_polvo;
    float desviacion_polvo;



// Configuracion del sensor de temperatura y humedad DHT11
#include <DHT.h>
#define DHTPIN 30              // Pin digital en el que se conecta el sensor DHT11
#define DHTTYPE DHT11          // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);      // Crea una instancia tipo dht para manejar el sensor
float temperatDHR;
float humedadDHR;
boolean error_DHT;



// Configuracion del sensor de presion y temperatura BMP085
Adafruit_BMP085 bmp;    // Crea una instancia tipo BMP085


// Configuracion del sensor de CO2  MG-811
// Comprado en http://sandboxelectronics.com/?product=mg-811-co2-gas-sensor-module
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
#define         BOOL_PIN                     (28)
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in 
                                                     //normal operation

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
//#define         ZERO_POINT_VOLTAGE           (0.475) //define the output of the sensor in volts when the concentration of CO2 is 400PPM. Ahora 0.492

// Calibrado de acuerdo con http://www.veetech.org.uk/Prototype_CO2_Monitor.htm
// A mayores incluye un ajuste din´mico del punto de aire limpio, implementado en la funcion MGRead
       // En caso de que el valor de tensi´n medida seamayor que el que corresponde a aire limpio (400ppm)
       // toma ese valor de tension como valor para aire limpio
       
// #define ZERO_POINT_VOLTAGE   (0.550) // Salida del sensor en aire libre: 400 ppm
// Tomo el valor por defecto, ya que las medidas en casa no son coherentes con le valor medido en el laboratorio.
float   ZERO_POINT_VOLTAGE   (0.455);  // Salida del sensor en aire libre: 400 ppm
#define TOTAL_POINT_VOLTAGE  (0.315) // Salida del sensor en aire saturado respirando: 40.000 ppm
float   REACTION_VOLTGAE   = ZERO_POINT_VOLTAGE - TOTAL_POINT_VOLTAGE;

/*****************************Globals***********************************************/
float    CO2Curve[3]  =  {2.602060, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE/(-2.0))};   
        //two points are taken from the curve with these two points, a line is formed 
        //which is "approximately equivalent" to the original curve.
        // data format:{ x, y, slope}; 
                // x (point1): (log(400)   = 2,602060), 
                // y (point2): (log(40000) = 4.602060) 
                // slope =     ( reaction voltage ) / (log(400) - log(40000)) 
float CO2Value;      // Concentracion de CO2 (ppm)
float volts_CO2;     // Valor de la tension medida en el detector de CO2
boolean error_CO2;


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
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
 
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
 
/**********************Application Related Macros**********************************/
#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)
 
/*****************************Globals***********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
float           rs = 0;
int   v_media = 0;


// Configuracion del Display
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


// Configuracion de Xively: Xively key to let you upload data
#define xivelyKey "seiuHmN24Nwtq8iOuVIUXGuF2oOSAKxEZHd5UzNZcjJOZz0g" // Bricolabs Xively/Cosm API key
#define FEED_ID 124735 // Bricolabs Xively/Cosm feed ID

/*
char* canales[7]      = {"Luminosity", "Temperature", "Pressure", "CO2", "DHT_Temp", "DHT_HR", "V_CO2"};
char* shortCanales[7] = {"Lumin", "Temp", "Pres", "CO2", "T_DHT", "HR", "V_CO2"};
char* unidades[7]     = {" %", " -C", " hPa", " ppm", "-C", "%", "mV"};

XivelyDatastream datastreams[] = {
  XivelyDatastream(canales[0], strlen(canales[0]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[1], strlen(canales[1]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[2], strlen(canales[2]), DATASTREAM_FLOAT),
  XivelyDatastream(canales[3], strlen(canales[3]), DATASTREAM_FLOAT),  
  XivelyDatastream(canales[4], strlen(canales[4]), DATASTREAM_FLOAT),  
  XivelyDatastream(canales[5], strlen(canales[5]), DATASTREAM_FLOAT),  
  XivelyDatastream(canales[6], strlen(canales[6]), DATASTREAM_FLOAT),  
};
*/



char* canales[8]      = {"Luminosidad", "Temperatura", "Presion", "CO2", "CO", "DHT_HR", "Part_Susp", "Notas"};
char* shortCanales[8] = {"Lumin", "Temp ", "Pres ", "CO2  ", "CO   ", "HR   ", "Psusp", "Notas"};
char* unidades[8]     = {" s/u", " -C ", " hPa", " ppm", " ppm", " %  ", " mg/m3", " __"};

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
  XivelyDatastream(canales[7], strlen(canales[7]), DATASTREAM_BUFFER, bufferValue, bufferSize),
  XivelyDatastream(stringId, DATASTREAM_STRING)
};





// wrap the datastreams into a feed
XivelyFeed feed(FEED_ID, datastreams, 9 /* number of datastreams */);

WiFiClient client;
XivelyClient xivelyclient(client);



// Configuracion de la conexion Wifi
/*
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
unsigned long time, time2;
int medida;
int ret;                        // se usa como control de respuesta en algunas llamadas a funciones


// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;


void setup() 
{
  Serial.begin(9600);        // Inicia protocolo de conexion serie
  bmp.begin();               // Inicia conexi´n con sensor bmp-811
  dht.begin();               // inicializa el sensor DHT

//  lcd.begin(16,2);
  lcd.begin(20,4);           // Inicia conexion con LCD
  
  time = time2 = millis();   // Inicia el intervalo de subida de datos

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
  


//  Ro = MQCalibration(MQ_PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");

  delay (5000);

}

    


void loop() {
  
  // Lee los valores del sensor DHT
  leeSensorDHT();
  
  // Lee los valores del sensor de CO2
  CO2Value = MGRead(MG_PIN);    
  
  // Lee el medidor de luminosidad LDR
  float lumValue   = map(analogRead(LUM_PIN) ,0,1023,0,100);

  // Lee los valores de presion y temperatura del BMP
  float tempValue  = bmp.readTemperature();
  float pressValue = bmp.readPressure() / 100.0;
  float altValue   = bmp.readAltitude();

  // Lee el medidor de CO
  float concentracion_LPG   = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
  float concentracion_CO    = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO);
  float concentracion_SMOKE = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE);

  
  
  // Lee la concentracion de polvo
  concentracion_polvo = concentracionPolvo ();
  desviacion_polvo    = desviacion_polvo;


  
  
  // Imprime los resultados
    Serial.print ( "CO2:          " );
    if (CO2Value == -1) Serial.print ( "< 400" );
    else Serial.print (CO2Value);
    Serial.print ( " ppm" );  
    Serial.print ( "\t ZPV = " );
    Serial.print (ZERO_POINT_VOLTAGE * 1000); 
    Serial.print ( "mV \t Tension medida = " );
    Serial.print (volts_CO2 * 1000); 
    Serial.print ( "mV " );
    Serial.print ( "\t(SEN-00007)" );
    Serial.println ();
 
    
    Serial.print ("Temperatura:   ");
    Serial.print (tempValue);
    Serial.print (" ºC");
    Serial.print (" \t\tBMP085");
    Serial.println ();
    
    Serial.print ("Presion:       ");
    Serial.print (pressValue);
    Serial.print (" mm");
    Serial.print (" \t\tBMP085");
    Serial.println ();
    
    Serial.print ("Temperatura:    ");
    Serial.print (temperatDHR);
    Serial.print (" ºC");
    Serial.print (" \t\tDHT11");
    Serial.println ();
    
    Serial.print ("Humedad rel:    ");
    Serial.print (humedadDHR);
    Serial.print (" %");
    Serial.print (" \t\tDHT11");
    Serial.println ();
    
    Serial.print ("GLP:            ");
    Serial.print (concentracion_LPG);
    Serial.print (" ppm");
    Serial.print (" \t\t Rs: ");
    Serial.print (rs);
    Serial.print ("\t(MQ-2) ");
    Serial.println ();

    Serial.print ("CO:             ");
    Serial.print (concentracion_CO);
    Serial.print (" ppm");
    Serial.print (" \t\t A3 = ");
    Serial.print (v_media);
    Serial.print (" \tRs: ");
    Serial.print (rs);
    Serial.print (" kohm\tRo: ");
    Serial.print (Ro);
    Serial.print ("kohm\t(MQ-2) ");
    Serial.println ();

    Serial.print ("HUMO:           ");
    Serial.print (concentracion_SMOKE);
    Serial.print (" ppm");
    Serial.print (" \t\t Rs: ");
    Serial.print (rs);
    Serial.print ("\t(MQ-2) ");
    Serial.println ();
   
    Serial.print ("Luminosidad:  ");
    Serial.print (lumValue);
    Serial.print (" (sin unidades) ");
    Serial.println ();
    
    Serial.print ("P susp:         ");
    Serial.print (concentracion_polvo);
    Serial.print (" mg/m3 \t desviacion: ");
    Serial.print (desviacion_polvo);
    Serial.print ("\t(VSP = ");
    Serial.print (V_sin_polvo);
    Serial.print (" V)");
    Serial.print ("\tSharp GP2Y1010AU0F ");
    Serial.println ();
 
    Serial.println ("----------");
    Serial.println ();

  
  // Da formato a los datos para el Datastream
  datastreams[0].setFloat (lumValue);
  datastreams[1].setFloat (tempValue);
  datastreams[2].setFloat (pressValue);
  datastreams[3].setFloat (CO2Value);
  datastreams[4].setFloat (concentracion_CO);
  datastreams[5].setFloat (humedadDHR);
  datastreams[6].setFloat (concentracion_polvo);
  
  datastreams[7].setBuffer ("sin novedad");
  
    // Pick a random number to send up in a string
  String stringValue("--");
 


  if (millis() > time + intervalo * 1000)  // Sube datos cada "intervalo" segundos.
  {
    medida = 1;
  }
  
  if (medida != 0)
  {
    medida = 0;
    time = millis(); 
   
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
  
  else if (millis() > time2 + 1000)     // Actualiza la pantalla LCD cada segundo.
  {
    int pendiente = intervalo - intervalo * (time2-time)/(intervalo * 1000);
    time2 = millis();
    
    int i = (pendiente / 5) % 7;    
    lcd.clear();

    lcd.setCursor(9, 4);
    lcd.print("faltan "); lcd.print(pendiente); lcd.print(" seg"); 
    
    lcd.setCursor(0, 0);
    lcd.print(shortCanales[i]); lcd.print (": "); lcd.print(datastreams[i].getFloat()); lcd.print(unidades[i]);
    lcd.setCursor(0, 1);
    lcd.print(shortCanales[(i+1)%7]); lcd.print (": "); lcd.print(datastreams[(i+1)%7].getFloat()); lcd.print(unidades[(i+1)%7]);
    lcd.setCursor(0, 2);
    lcd.print(shortCanales[(i+2)%7]); lcd.print (": "); lcd.print(datastreams[(i+2)%7].getFloat()); lcd.print(unidades[(i+2)%7]);
    
    
    float concentra = datastreams[4].getFloat();
    Serial.println (concentra);
    
    delay (2000);
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
  
  float medida = 0.0;
  volts_CO2 = 0;
  error_CO2 = false;
  
  for (int i=0; i < READ_SAMPLE_TIMES; i++) 
  {
    medida += analogRead (mg_pin);
    delay (READ_SAMPLE_INTERVAL);
  }
  
  medida = medida / READ_SAMPLE_TIMES;     // Calcula la media de las medidas
  medida = medida * 5 / 1024;              // Convierte a mV
  volts_CO2 = medida / DC_GAIN;             // vots_CO2 es el valor de tensi´n en el sensor
    

     // Estas lineas son un ajuste en caso de que se mide un valor mayor que ZERO_POINT_VALUE
     // que corresponderia a un aire mas limpio que 400 ppm. En ese caso se ajusta ZERO_POINT_VALUE 
     // vuelve a valcular la curva de ajuste, y no se sube el valor medido.
  if (volts_CO2 > ZERO_POINT_VOLTAGE)
  {
    float volt_ant = ZERO_POINT_VOLTAGE;
    
    ZERO_POINT_VOLTAGE = volts_CO2 * 1.001;
    REACTION_VOLTGAE   = ZERO_POINT_VOLTAGE - TOTAL_POINT_VOLTAGE;
    CO2Curve[1]  =  ZERO_POINT_VOLTAGE;
    CO2Curve[2]  =  REACTION_VOLTGAE / (-2.0);
 
    Serial.print ("        *** Actualizado ZERO_POINT_VOLTAGE. Valor anterior = ");
    Serial.print (volt_ant * 1000);
    Serial.print (" mV \tNuevo valor = ");
    Serial.print (ZERO_POINT_VOLTAGE * 1000);
    Serial.println (" mV    ***");
    error_CO2 = true;
  }

  
  float concentracion = float (MGGetPercentage(volts_CO2, CO2Curve));

  return concentracion;  
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
   if (volts >= ZERO_POINT_VOLTAGE) 
   {
      return -1;
   }
   else if ((volts) <  TOTAL_POINT_VOLTAGE) 
   {
      return 0;
   }

   else {
     long ppm = pow(10, (volts - pcurve[1]) / pcurve[2] + pcurve[0]);
     return float (ppm);
   }
}



float leeSensorDHT()
{
  // Lee los valores del sensor DHT

  int respuesta = 0;
  error_DHT = false;
  
  temperatDHR = dht.readTemperature();
  humedadDHR  = dht.readHumidity();

  if (isnan(temperatDHR) || isnan(humedadDHR))
  {
    respuesta = -1;
    Serial.println ("error en la lectura del sensor DHT");
    error_DHT = true;
  }
  else
  {
    respuesta = 0;
    
    /*
    Serial.print ("Temperatura DHR = ");
    Serial.print (temperatDHR);
    Serial.print ("/t Humedad DHR = ");
    Serial.print (humedadDHR);
    Serial.print (" %");
    Serial.println ();
    Serial.println ();
    */
  }
  
  return respuesta;
}


float valorCO(int pin_CO)
{
  int lectura = 0;
  float concentracion;

  for (int i = 0; i < 5; i++) 
  {
    lectura += analogRead(pin_CO);
    delay(50);
  }
  
  concentracion = float (lectura) / 5.0;  // Calcula la media de las medidas

  return concentracion;  
}













float concentracionPolvo ()
{
  int samplingTime = 280;
  int deltaTime = 40;
  int sleepTime = 9680;
  int valor_medido;
  float medida [100];
  float V_media;
  
  pinMode (DUST_PIN_O, OUTPUT);

  for (int i = 0; i < 100; i++)
  {
    digitalWrite(DUST_PIN_O, LOW); // power on the LED
    delayMicroseconds(samplingTime);
  
      valor_medido = analogRead(DUST_PIN_I); // read the dust value

    delayMicroseconds(deltaTime);
    digitalWrite(DUST_PIN_O, HIGH); // turn the LED off
    delayMicroseconds(sleepTime);
  }
  
  digitalWrite(DUST_PIN_O, HIGH); // turn the LED off

  
  concentracion_polvo = 0.0;       // mg/m3
  V_media = 0.0;
  for (int i = 0; i < 100; i++)
  {
    // 0 - 5V mapped to 0 - 1023 integer values
    // recover voltage
       medida [i] = valor_medido * (5.0 / 1024.0);
    V_media += medida [i];
   
    // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
    // Chris Nafis (c) 2012
       medida [i] = ganancia_polvo * (medida [i] - V_sin_polvo);

    concentracion_polvo += medida [i];
  }

  V_media = V_media / 100;
  if (V_media < V_sin_polvo) 
  {
    float V_anterior = V_sin_polvo;
    V_sin_polvo = V_media - 0.01;   
    
    concentracion_polvo = 0;
    
    Serial.print ("        *** Actualizado V_sin_Polvo. Valor anterior = ");
    Serial.print (V_anterior);
    Serial.print (" V \tNuevo valor = ");
    Serial.print (V_sin_polvo);
    Serial.println (" V    ***");
    
    // error_susp = true;
  }

    
  
  concentracion_polvo = concentracion_polvo / 100;
  
  
  desviacion_polvo;  
  for (int i = 0; i < 100; i++)
  {
    desviacion_polvo += (medida [i] - concentracion_polvo) * (medida [i] - concentracion_polvo);
  }
  desviacion_polvo = pow ((desviacion_polvo / 100), 0.5);
      
  return concentracion_polvo;

}










/*

float leeMQ_2 ()
{
  float concentracion = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
  Serial.print ("\t LPG: "); 
  Serial.print (concentracion );
  Serial.print (" ppm");

  float concentracion_CO = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO);
  Serial.print ("\t\tCO: "); 
  Serial.print (concentracion_CO );
  Serial.print (" ppm \t");

  concentracion = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE);
  Serial.print ("\t\tSMOKE: "); 
  Serial.print (concentracion );
  Serial.print (" ppm");
  Serial.println ();

  return concentracion_CO;
}
*/

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}
 
/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
 
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
 
  return val; 
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  int medida,rs_calculada;
  v_media = 0;
  rs = 0;
 
   for (i=0;i<READ_SAMPLE_TIMES;i++) {
   medida = analogRead(mq_pin);
   rs_calculada = MQResistanceCalculation(medida);
   rs += rs_calculada;
   
   v_media += medida;
 
 /*   
   Serial.print ("\t\t Valor tesion medido: ");
   Serial.print (medida);
   Serial.print ("\t Valor de rs medido: ");
   Serial.print (rs_calculada);
   Serial.print ("\t Ro: ");
   Serial.print (Ro);
   Serial.println ();

   delay(500);
 */
 
   delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;
  
  v_media = v_media / READ_SAMPLE_TIMES;
  delay (500); 

/* 
  Serial.print ("\t \t Valor de V medio: ");
  Serial.print (v_media);
  Serial.print ("\t Valor de rs medio: ");
  Serial.print (rs);
  Serial.print ("\t Ro: ");
  Serial.print (Ro);
  Serial.println ();
*/

  return rs;  
}
 
/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which 
         calculates the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
 
  return 0;
}
 
/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  
  /*
  Serial.print ("\t ratio: ");
  Serial.print (rs_ro_ratio);
  Serial.println ();
  */
  
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
