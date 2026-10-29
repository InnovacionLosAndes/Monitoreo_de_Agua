#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

String serialBuffer = "";
#define RX2_PIN 16
 
 
float calibration_value = 15.00 - 0.31;
 
int pHsensor = 34;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;
 void display_pHVal();
float ph_act;
unsigned long prevMillis = 0;
const long interval = 1000;
const char* ssid = "(YOUR_SSID)";
const char* password = "(YOUR_PASSWORD)";

WebServer server(80);

IPAddress ip(192, 168, 2, 108);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void handleRoot() {

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>MONITOR DE CALIDAD DE AGUA</title>";
  html += "<style>body { font-family: times; background: #222; color: #00d5ff; padding: 20px; }";
  html += ".terminal { background: #928E85; border: 1px solid #0f0; padding: 15px; min-height: 200px; white-space: pre-wrap; font-size: 17px; }</style>";
  html += "<meta http-equiv='refresh' content='1'>"; 
  html += "</head><body>";
  html += "<h1>Sistema de Monitoreo de Calidad del Agua</h1>";
  html += "<p>COMPUTANDO:</p>";
  html += "<div class='terminal'>" + serialBuffer +  "</div>";
  html += String("<div class='terminal'>") + "<p>Valor de PH:</p>" + String(ph_act) + "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin();
  pinMode(pHsensor, INPUT);

  Serial2.begin(115200, SERIAL_8N1, RX2_PIN, 17);
  
  WiFi.mode(WIFI_STA);
  if (!WiFi.config(ip, gateway, subnet)) {
    Serial.println("Error al configurar IP Estática");
  }
  
  WiFi.begin(ssid, password);
  Serial.println("\nConectando");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("\nConectado a la Red WIFI");
  Serial.print("Direccion IP Local del microcontrolador: ");
  Serial.println(WiFi.localIP());
  

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  

  while (Serial2.available() > 0) {
    char c = Serial2.read();
    serialBuffer += c;
  
    if (serialBuffer.length() > 2000) {
      serialBuffer = serialBuffer.substring(500); 
    }
  }
 
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis >= interval) {
    prevMillis = currentMillis;


    for(int i=0; i<10; i++) { 
      buffer_arr[i] = analogRead(pHsensor); 
      delay(10); 
    } 


    for(int i=0; i<9; i++) { 
      for(int j=i+1; j<10; j++) { 
        if(buffer_arr[i] > buffer_arr[j]) { 
          temp = buffer_arr[i]; 
          buffer_arr[i] = buffer_arr[j]; 
          buffer_arr[j] = temp; 
        } 
      } 
    } 

    avgval = 0; 
    for(int i=2; i<8; i++) {
      avgval += buffer_arr[i]; 
    }

    float volt = (float)avgval * 5 / 4095.0 / 6; 
    ph_act = -4.90 * volt + calibration_value; 

    Serial.print("Valor pH: "); 
    Serial.println(ph_act); 
  }
}
