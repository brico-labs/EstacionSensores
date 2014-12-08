long  MGGetPercentage(float volts, float *pcurve)
{
   if (volts >= ZERO_POINT_VOLTAGE) 
   {
      return -1;
   }
   else if ((volts) <  TOTAL_POINT_VOLTAGE) 
   {
      return -2;
   }

   else {
     long ppm = pow(10, (volts - pcurve[1]) / pcurve[2] + pcurve[0]);
     return float (ppm);
   }
}

