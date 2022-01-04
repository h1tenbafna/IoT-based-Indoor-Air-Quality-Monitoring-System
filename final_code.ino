#include <SoftwareSerial.h>
#define RX 2
#define TX 3
String apiKey = "AXVB623U7WKU3OGT";  //"Write API key" 5FIDX4YWTJIAUDYJ


        /* MQ135 Gas Sensor*/

int gas_sensor = A0; // MQ135 Sensor pin 
float m = -0.3376; //Slope 
float b = 0.7165; //Y-Intercept 
float R0 = 20.95; //Sensor Resistance in fresh air after calibration (22.95)


      /* MQ7 CO Sensor*/

int CO_sensor = A1; // MQ7 pin 
float m1 = -0.6527; //Slope 
float b1 = 1.30; //Y-Intercept 
float R01 = 0.41; //Sensor Resistance in fresh air after calibration 0.17-0.51

SoftwareSerial esp8266(RX,TX); 

      /* dht11 temp. & humidity Sensor*/
#include "dht.h"
#define DHT11_PIN A2
dht DHT;

void setup() {
  
 
  Serial.begin(9600);      // PC to Arduino Serial Monitor

  esp8266.begin(115200);   // Arduino to ESP01 

  pinMode(gas_sensor, INPUT); // For MQ135
  pinMode(CO_sensor,INPUT);  // For MQ7
  pinMode(A2,INPUT);      //For DHT Sensor
 } 

void loop() { 

  float sensor_volt; //variable for sensor voltage 
  float RS_gas; //variable for sensor resistance  
  float ratio; //variable for ratio
  float sensorValue = analogRead(gas_sensor); //Read analog values of sensor  
  sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 
  RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
  Serial.print(" RS VALUE = ");
  Serial.println(RS_gas);
  ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
  double ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value  
  double ppm = pow(10, ppm_log); //Convert ppm value to log scale 
  Serial.print(" PPM LEVEL = ");
  Serial.println(ppm);
  

  float sensor_volt1; //Define variable for sensor voltage 
  float RS_gas1; //Define variable for sensor resistance  
  float ratio1; //Define variable for ratio
  float sensorValue1 = analogRead(CO_sensor); //Read analog values of sensor  
  sensor_volt1 = sensorValue1*(5.0/1023.0); //Convert analog values to voltage 
  RS_gas1 = ((5.0*10.0)/sensor_volt1)-10.0; //Get value of RS in a gas
  Serial.print(" RS VALUE = ");
  Serial.println(RS_gas1);
  ratio1 = RS_gas1/R01;  // Get ratio RS_gas/RS_air
  double ppm_log1 = (log10(ratio1)-b1)/m1; //Get ppm value in linear scale according to the the ratio value  
  double ppm1 = pow(10, ppm_log1); //Convert ppm value to log scale 
  Serial.print("CO PPM = ");
  Serial.println(ppm1);


  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  float temp = DHT.temperature;
  Serial.println(temp);
  float hum = DHT.humidity;
  Serial.print("Humidity = ");
  Serial.println(hum);



  esp8266.println("AT+CIPMUX=0\r\n");      // To Set MUX = 0
  delay(2000);                             // Wait for 2 sec

  // TCP connection 
  String cmd = "AT+CIPSTART=\"TCP\",\"";   // TCP connection with https://thingspeak.com server
  cmd += "184.106.153.149";                // IP add of api.thingspeak.com
  cmd += "\",80\r\n\r\n";                  // Port No. = 80

  esp8266.println(cmd);                    // Display above Command on PC
  Serial.println(cmd);                     // Send above command to Rx1, Tx1

  delay(1000);                         

  if(esp8266.find("ERROR"))                // checking error in TCP connection
  { 
    Serial.println("AT+CIPSTART error");   // Display error msg to PC

  }

  // preparing GET string 
  String getStr = "GET /update?api_key=";   
  getStr += apiKey;
  getStr +="&field1=";
  getStr += ppm;
  getStr +="&field2=";
  getStr += ppm1; 
  getStr +="&field3=";
  getStr += temp;
  getStr +="&field4=";
  getStr += hum;
  getStr += "\r\n\r\n"; 

  Serial.println(getStr);                 // Display GET String on PC

  cmd = "AT+CIPSEND=";                    // send data length 
  cmd += String(getStr.length());
  cmd+="\r\n";

  Serial.println(cmd);                   // Display Data length on PC
  esp8266.println(cmd);                  // Send Data length command to Tx1, Rx1
  if(esp8266.find(">"))                    // verify connection with cloud
  {
    Serial.println("Pushed data to the CLOUD");  

    esp8266.print(getStr);                 // Send GET String to Rx1, Tx1
  }
  else
  { 
    esp8266.println("AT+CIPCLOSE\r\n");    // Send Close Connection command to Rx1, Tx1
    Serial.println("AT+CIPCLOSE");         // Display Connection closed command on PC
  } 
  
  delay(15000);                            // wait for 15 sec
 }
