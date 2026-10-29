#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 12
#define TdsSensorPin A2
#define VREF 5.0
#define SCOUNT 30

void(* resetFunc) (void) = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
bool bufferListo = false;

float averageVoltage = 0, tdsValue = 0, temperature = 25.0;

unsigned long analogSampleTimepoint = 0;
unsigned long printTimepoint = 0;
unsigned long tempSampleTimepoint = 0;

int getMedianNum(int bArray[], int iFilterLen);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(TdsSensorPin, INPUT);

  sensors.begin();
  sensors.setWaitForConversion(false); 
  sensors.requestTemperatures();      
  
  analogSampleTimepoint = millis();
  printTimepoint = millis();
  tempSampleTimepoint = millis();
}

void loop() {

  if (Serial.available() > 0) { 
    

    String command = Serial.readString(); 
    
  
    command.trim(); 
    
    
    if (command == "RESET") { 
      Serial.println("REINICIANDO POR COMANDO SERIAL PRIORITARIO");
      delay(1000);
      Serial.println("PROCESANDO");
      delay(1000);
      resetFunc(); 
    }
  }


   if (millis() - tempSampleTimepoint > 1000U) {
    tempSampleTimepoint = millis();
    float tempLectura = sensors.getTempCByIndex(0);
    
    if (tempLectura != DEVICE_DISCONNECTED_C) {
      temperature = tempLectura;
    }
    
    sensors.requestTemperatures(); 
    Serial.print("Temperatura: ");
    Serial.print(temperature, 1);
    Serial.println(" °CELSIUS");
     Serial1.print("Temperatura: ");
    Serial1.print(temperature, 1);
    Serial1.println(" °CELSIUS");
  
  }

 
  if (millis() - analogSampleTimepoint > 120U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
      bufferListo = true; 
    }
  }


  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    
    if (bufferListo) { 
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      }
      
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
      
  
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      float compensationVolatge = averageVoltage / compensationCoefficient;
      
      tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge 
                  - 255.86 * compensationVolatge * compensationVolatge 
                  + 857.39 * compensationVolatge) * 0.5;
      
    
      Serial.print("Valor Total de Solidos Disueltos: ");
      Serial.print(tdsValue, 0);
      Serial.println(" ppm");

      Serial1.print("Valor Total de Solidos Disueltos: ");
      Serial1.print(tdsValue, 0);
      Serial1.println(" ppm");
    } else {
    
      Serial.println("Computando TDS");
       Serial1.println("Computando TDS");
    }
 
    Serial.println("Computando");
       Serial1.println("Computando");
  }
}


int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) bTemp = bTab[(iFilterLen - 1) / 2];
  else bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

