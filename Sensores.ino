/*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN-000007
Remarks: This function reads the output of SEN-000007
************************************************************************************/ 
float MGRead(int mg_pin)
{
  #define         DC_GAIN                      (8.5)   //define the DC gain of amplifier
  /***********************Software Related Macros************************************/
  #define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
  #define         READ_SAMPLE_TIMES            (2)     //define the time interval(in milisecond) between each samples in 
  
  float    CO2Curve[3]  =  {2.602060, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE/(-2.0))};   
        //two points are taken from the curve with these two points, a line is formed 
        //which is "approximately equivalent" to the original curve.
        // data format:{ x, y, slope}; 
                // x (point1): (log(400)   = 2,602060), 
                // y (point2): (log(40000) = 4.602060) 
                // slope =     ( reaction voltage ) / (log(400) - log(40000)) 
                
  float medida = 0.0;
  float concentracionCO2;
  volts_CO2 = 0;
  boolean error_CO2 = false;
  
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
 
    Serial.print ("        *** Actualizado ZERO_POINT_VOLTAJE. Valor anterior = ");
    Serial.print (volt_ant * 1000);
    Serial.print (" mV \tNuevo valor = ");
    Serial.print (ZERO_POINT_VOLTAGE * 1000);
    Serial.println (" mV    ***");
    error_CO2 = true;
    
    concentracionCO2 = -100;
    textos_medidas [3] = "Ajustando";

  }
  else if ((volts_CO2) <  TOTAL_POINT_VOLTAGE) 
  {
    concentracionCO2 = -2;

    textos_medidas [3] = "Exceso";
  }

  else 
  {
    concentracionCO2 = pow(10, (volts_CO2 - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
    textos_medidas [3] = "";

  }
  
  //float concentracion = float (MGGetPercentage(volts_CO2, CO2Curve));

  return concentracionCO2;  
}




/*****************************                   **********************************
Input:   
         
Output:  
Remarks: 
         
************************************************************************************/ 
float leeSensorDHT()
{
  // Lee los valores del sensor DHT

  int respuesta = 0;
  error_DHT = false;
  
  temperatDHR = dht.readTemperature();
  humedadDHR  = dht.readHumidity();

  if (isnan(temperatDHR) || isnan(humedadDHR))
  {
    respuesta = -100;
    Serial.println ("error en la lectura del sensor DHT");
    error_DHT = true;
    textos_medidas [5] = "Error";

  }
  else
  {
    respuesta = 0;
    textos_medidas [5] = "";
    
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








/************************  Medidosr de suciedad en el aire *************************
Modelo:  Sharp GP2Y1010AU0F

Metodo:  Se realiza una serie de medidas para detectar la uniformidad de la misma
         de acuerdo con la nota de aplicacion: 
         http://sharp-world.com/products/device/lineup/data/pdf/datasheet/gp2y1010au_appl_e.pdf
Docume:  http://www.sharpsme.com/optoelectronics/sensors/air-sensors/GP2Y1010AU0F
************************************************************************************/ 

float concentracionPolvo ()
{
  int samplingTime = 280;
  int deltaTime = 40;
  int sleepTime = 9680;
  
  boolean medida_valida = true;
  int   valor_medido   [NUMERO_MED_PSUSP];
  float concent_medido [NUMERO_MED_PSUSP];
  float concent_media   = 0.0;
  float concent_desvia  = 0.0;
  
  // Toma una secuencia de medidas de acuerdo con la hoja de caracteristicas  
  pinMode (DUST_PIN_O, OUTPUT);
  
  for (int i = 0; i < NUMERO_MED_PSUSP; i++)
  {
    digitalWrite(DUST_PIN_O, LOW);               // power on the LED
    delayMicroseconds(samplingTime);
  
      valor_medido [i] = analogRead(DUST_PIN_I); // read the dust value

    delayMicroseconds(deltaTime);
    digitalWrite(DUST_PIN_O, HIGH);              // turn the LED off
    delayMicroseconds(sleepTime);
  }  
  digitalWrite(DUST_PIN_O, HIGH);                // turn the LED off


  // Calcula la concentracion asociada a cada media y la media
  for (int i = 0; i < NUMERO_MED_PSUSP; i++)       // Calcula la desviaci´n 
  {
    concent_medido [i] = ganancia_polvo * (valor_medido [i] * 5.0 / 1024.0  - V_sin_polvo);
    concent_media += concent_medido [i];
  }
  concent_media = concent_media / NUMERO_MED_PSUSP;


    
  // Comprueba si la media es menor que V_sin_polvo. 
  // En ese caso toma ese valor como nuevo V_sin polvo y descarta la medida.
  if (concent_media < 0)
  {
    Serial.print (" ajuste de V_sin Polvo. V anterior = ");
    Serial.print (V_sin_polvo);
  
      V_sin_polvo += concent_media / ganancia_polvo;

    Serial.print (" Nuevo valor = ");
    Serial.print (V_sin_polvo);
    Serial.println  ();

    medida_valida = false;
    
    concentracion_polvo = - 100;
    
    textos_medidas [6] = "Ajustando";
  }
    
  else
  {
    for (int i = 0; i < NUMERO_MED_PSUSP; i++)       // Calcula la desviaci´n 
    {
      concent_desvia = (concent_medido [i] - concent_media) * (concent_medido [i] - concent_media);
    }
    concent_desvia = pow ((concent_desvia / NUMERO_MED_PSUSP), 0.5);
    concentracion_polvo = concent_media;
    desviacion_polvo    = concent_desvia;
    textos_medidas [6] = "";
  }
  
  return concentracion_polvo;

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

  float RL_VALUE            = 5.0;  //define the load resistance on the board, in kilo ohms
  float RO_CLEAN_AIR_FACTOR = 9.83; //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
  int medida,rs_calculada;
  float rs_medido;
  float rs_media = 0;
  float rs_r0_ratio;

        float r0 = Ro;
  
  // Realiza varias medidas de la resietencia del sensor y calcula la media
  for (int i = 0; i < READ_SAMPLE_TIMES; i++) 
  {
    medida = analogRead(mq_pin);
    rs_medido = float (RL_VALUE * (1023 - medida) / medida);
    rs_media  += rs_medido;
    delay(READ_SAMPLE_INTERVAL);
  }
  rs_media = rs_media / READ_SAMPLE_TIMES;

  rs_r0_ratio = rs_media / r0;
  
  // Segun la tabla de caracteristicas del sensor, en aire limpio, rs/r0 es 
        //casi constante e igual a 10. En caso de que ese valor se exceda, se entiende
        // que r0 esta mal calculado y debe reajustarse
  if (rs_r0_ratio > 10.0)
  {
    Serial.print (" hay que reajustar r0. r0 anterior = "); Serial.print (r0);
    r0 = rs_media / 10.0;
    rs_r0_ratio = rs_media / r0;

    Ro =r0;
    Serial.print ("\tr0 nuevo = "); Serial.print (r0);
    Serial.println ();
    
    textos_err_CO2 = "Ajuste rO";
    concentracion_CO = -101;
  }
    rs = rs_media;
  
  CALIBARAION_SAMPLE_TIMES --;
  if (CALIBARAION_SAMPLE_TIMES > 0)
  {
    textos_err_CO2 = "Calibrando";
    concentracion_CO = -102.0;
  }
  else
  {
    CALIBARAION_SAMPLE_TIMES = 0;
    // Los valores de la curva de de respuesta del sensor estan calculados
          // a partir de la tabla de caracteristicas del sensor

    concentracion_CO = pow(10, (4.5542 - 3.1814 * log10 (rs_r0_ratio)));

    textos_err_CO2 = "Ok";
        
  }


      Serial.print ("\t valor de la medida ");
      Serial.print (concentracion_CO);
      Serial.print ("\t texto origen **");
      Serial.print (textos_err_CO2);
      Serial.print ("**\t CST = ");
      Serial.print (CALIBARAION_SAMPLE_TIMES);
      Serial.println ();
      delay (20);

  return concentracion_CO;

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
  /*int i;
  float val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
 
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
 */
  return 3; 
}





String float_to_text (float numero, int decimales)
{
  String valor_texto;
  
  valor_texto = String (long(numero)) + String (".");
  if (long(numero * 100) % 100 < 10) valor_texto += "0";
  valor_texto +=  String ( long(numero * 100) % 100);
  
  return valor_texto;
}

