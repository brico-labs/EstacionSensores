

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



void printValues()
{
  if (imprime = true)            // Imprime los valores actualizados
    {
      Serial.print ( "CO2:           " );
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
    
      Serial.print ("Presion:      ");
      Serial.print (pressValue);
      Serial.print (" mm");
      Serial.print (" \t\tBMP085");
      Serial.println ();
    
      Serial.print ("Temperatura:     ");
      Serial.print (temperatDHR);
      Serial.print (" ºC");
      Serial.print (" \t\tDHT11");
      Serial.println ();
    
      Serial.print ("Humedad rel:    ");
      Serial.print (humedadDHR);
      Serial.print (" %");
      Serial.print (" \t\tDHT11");
      Serial.println ();
    
      Serial.print ("CO:             ");
      Serial.print (concentracion_CO);
      Serial.print (" ppm");
      Serial.print (" \tRs: ");
      Serial.print (rs);
      Serial.print (" kohm\tRo: ");
      Serial.print (Ro);
      Serial.print ("kohm\t(MQ-2) ");
      Serial.println ();
    
      Serial.print ("Luminosidad:    ");
      Serial.print (lumValue);
      Serial.print (" (sin unidades) ");
      Serial.println ();
    
      Serial.print ("P susp:          ");
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
      
    }
}

