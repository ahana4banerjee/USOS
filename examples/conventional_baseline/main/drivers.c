int DHT_Read(float *temperature)         { *temperature = 26.5f; return 0; }
int BMP280_ReadPressure(float *p_hpa)    { *p_hpa = 1013.25f;  return 0; }
int HCSR04_ReadDistance(float *dist_cm)  { *dist_cm = 45.0f;   return 0; }
int LDR_ReadRaw(float *adc_raw)          { *adc_raw = 2048.0f; return 0; }
