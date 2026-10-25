#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

const uint32_t TIMEOUT = 500UL;
String light_intensity;
#define LIGHT_SENSOR_PIN 34
#define rainAnalog 39
#define rainDigital 14
#define MQ135_THRESHOLD_1 1000 
#define sensorPower 4
#define sensorPin 36

HTTPClient http;
int val = 0;
#define RE 32
#define DE 33
const int refresh =3;
//const byte code[]= {0x01, 0x03, 0x00, 0x1e, 0x00, 0x03, 0x65, 0xCD};
const byte nitro[] =       {0x06, 0x03, 0x00, 0x04, 0x00, 0x01, 0xc4, 0x7c};
const byte phos[] =        {0x06, 0x03, 0x00, 0x05, 0x00, 0x01, 0x95, 0xbc};
const byte pota[] =        {0x06, 0x03, 0x00, 0x06, 0x00, 0x01, 0x65, 0xbc};
const byte soil_ph[] =     {0x06, 0x03, 0x00, 0x03, 0x00, 0x01, 0x75, 0xbd};
const byte soil_moist[] =  {0x06, 0x03, 0x00, 0x00, 0x00, 0x01, 0x85, 0xbd};
const byte soil_temp[] =   {0x06, 0x03, 0x00, 0x01, 0x00, 0x01, 0xd4, 0x7d};
const byte soil_cond[] =   {0x06,0x03, 0x00, 0x02, 0x00, 0x01, 0x24, 0x7d};

byte values[11];
SoftwareSerial mod(26,27);
const char* ssid     = "YOUR_WIFI_SSID";      // redacted
const char* password = "YOUR_WIFI_PASSWORD";  // redacted
const char* serverName = "https://your-server.example/sm.php"  /* redacted */;
String apiKeyValue = "YOUR_API_KEY";  // redacted
// Set web server port number to 80
WiFiServer server(80);
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;
String waterlevel;
String rainfallstatus;
 
void setup() {
  Serial.begin(4800);
  mod.begin(4800);
  lcd.begin(16,2);
  pinMode(rainDigital,INPUT);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  
  lcd.print("Initializing....");
  delay(1000);
  lcd.clear();
  
  Serial.println(F("BME280 test"));
  lcd.print("BME testing");

  bool status;

  // default settings
 

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
  lcd.clear();

  Serial.print("Connecting to ");
  lcd.print(" connecting to");
  lcd.setCursor(5,1);
  lcd.print(ssid);
  
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  lcd.clear();
  Serial.println("WiFi connected.");
  lcd.print("Wifi Connected");
  delay(500);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("N:");
  lcd.setCursor(6,0);
  lcd.print("P:");
  lcd.setCursor(12,0);
  lcd.print("K:");
  lcd.setCursor(0,1);
  lcd.print("pH:");
  lcd.setCursor(4,1);
  lcd.print(" %M:");
  lcd.setCursor(10,1);
  lcd.print(" *C:");
}
 
void loop() {
  
    int analogValue = analogRead(LIGHT_SENSOR_PIN);

  Serial.print("Analog Value = ");
  Serial.print(analogValue);   // the raw analog reading

  // We'll have a few threshholds, qualitatively determined
  if (analogValue < 40) {
    light_intensity="dark";
    Serial.println(" => Dark");
  } else if (analogValue < 800) {
    light_intensity="dim";
    Serial.println(" => Dim");
  } else if (analogValue < 2000) {
    light_intensity="light";
    Serial.println(" => Light");
  } else if (analogValue < 3200) {
    light_intensity=" bright";
    Serial.println(" => Bright");
  } else {
    light_intensity="very bright";
    Serial.println(" => Very bright");
  }

  
  int rainAnalogVal = analogRead(rainAnalog);
  int rainDigitalVal = digitalRead(rainDigital);
  if(rainDigitalVal==0){
    rainfallstatus="raining";
  }else{
    rainfallstatus="not raining";
  }

  Serial.print(rainAnalogVal);
  Serial.print("\t");
  Serial.println(rainDigitalVal);
  
  int MQ135_data = analogRead(35);
  if(MQ135_data < MQ135_THRESHOLD_1){
  Serial.print("Fresh Air: ");
  }
  else {
  Serial.print("Poor Air: ");
  }
  Serial.print(MQ135_data);
  Serial.println(" PPM");
  
  
  printValues();
  
  delay(delayTime);
  digitalWrite(sensorPower, HIGH);
  delay(10);
  val = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);
  Serial.print("Water level: ");
  Serial.println(val);
  if(val<1200){
    waterlevel="empty";
  }
  else if(val>1200 && val<1800){
    waterlevel= "low";
  }
  else if(val>2100){
    waterlevel="high";
  }
  else if(val>1800 && val<2100){
    waterlevel= "medium";
  }
  Serial.print("Water level: ");
  Serial.println(waterlevel);

  byte val1,val2,val3,val4,val5,val6,val7;
  val1 = nitrogen();
  delay(250);
  val2 = phosphorous();
  delay(250);
  val3 = potassium();
  delay(250);
  val4 = ph();
  delay(250);
  val5 = moisture();
  delay(250);
  val6 = temp();
  delay(250);
  val7 = EV();
  delay(250);
  

  Serial.print("Nitrogen:    ");
  Serial.print(val1);
  Serial.println(" mg/kg");
  Serial.print("Phosphorous: ");
  Serial.print(val2);
  Serial.println(" mg/kg");
  Serial.print("Potassium:   ");
  Serial.print(val3);
  Serial.println(" mg/kg");
  Serial.print("PH:          ");
  Serial.print(val4/10.0);
  Serial.println();
  Serial.print("Moisture:    ");
  Serial.print(val5);
  Serial.println(" %");
  Serial.print("soil_temp:   ");
  Serial.print(val6/10.0);
  Serial.println("*C");
  Serial.print("soil_cvt:    ");
  Serial.print(val7);
  Serial.println(" us/cm");

  
  lcd.setCursor(0,0);
  lcd.print("N:");
  lcd.print(val1);
  lcd.setCursor(6,0);
  lcd.print("P:");
  lcd.print(val2);
  lcd.setCursor(11,0);
  lcd.print("K:");
  lcd.print(val3);
  lcd.setCursor(0,1);
  lcd.print("pH:");
  lcd.print(val4/10);
  lcd.setCursor(4,1);
  lcd.print(" %M:");
  lcd.print(val5);
  lcd.setCursor(10,1);
  lcd.print(" *C:");
  lcd.print(val6/10);
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println(" <meta http-equiv ='refresh'content='");
            client.println(String(refresh));
            client.println("'/>\n");
            // CSS to style the table 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // Web Page Heading
            client.println("</style></head><body><h1>Edelsmafa</h1>");
            client.println("<table><tr><th> Hurudza Readings </th><th>VALUE</th></tr>");
            client.println("<tr><td>Nitrogen       N:  </td><td><span class=\"sensor\">");
            client.println(val1);
            client.println(" mg/kg</span></td></tr>");  
            client.println("<tr><td>Phosphorus     P:  </td><td><span class=\"sensor\">");
            client.println(val2);
            client.println(" mg/kg </span></td></tr>");       
            client.println("<tr><td>Potasium       K:  </td><td><span class=\"sensor\">");
            client.println(val3);
            client.println(" mg/kg </span></td></tr>");
            client.println("<tr><td>soil_ph        pH:  </td><td><span class=\"sensor\">");
            client.println(val4/10);
            client.println(" </span></td></tr>"); 
            client.println("<tr><td>soil_Humidity  M: </td><td><span class=\"sensor\">");
            client.println(val5/10);
            client.println(" %</span></td></tr>"); 
            client.println("<tr><td>soil_temp      T: </td><td><span class=\"sensor\">");
            client.println(val6/10);
            client.println(" *C</span></td></tr>"); 
            client.println("<tr><td>soil_ctvt     EV: </td><td><span class=\"sensor\">");
            client.println(val7);
            client.println(" us/cm </span></td></tr>"); 
            client.println("<tr><td>environ_Temp  T*C: </td><td><span class=\"sensor\">");
            client.println(bme.readTemperature());
            client.println(" *C </span></td></tr>"); 
            client.println("<tr><td>environ_Humid M%: </td><td><span class=\"sensor\">");
            client.println(bme.readHumidity());
            client.println("% </span></td></tr>"); 
            client.println("<tr><td>Pressure      P: </td><td><span class=\"sensor\">");
            client.println(bme.readPressure()/100.0F);
            client.println("hPa </span></td></tr>"); 
            client.println("<tr><td>Altitude      H: </td><td><span class=\"sensor\">");
            client.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
            client.println(" m </span></td></tr>");
            client.println("<tr><td>Water level   : </td><td><span class=\"sensor\">");
            client.println(waterlevel);
            client.println(" </span></td></tr>");
            client.println("<tr><td>Air quality   : </td><td><span class=\"sensor\">");
            client.println(MQ135_data);
            client.println(" PPM </span></td></tr>");
            client.println("<tr><td>Rainfall status   : </td><td><span class=\"sensor\">");
            client.println(rainfallstatus);
            client.println("  </span></td></tr>");
            client.println(rainAnalogVal);
            client.println("<tr><td>light_intensity   : </td><td><span class=\"sensor\">");
            client.println(light_intensity);
            client.println(" </span></td></tr>");
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop

            ////server test code

            HTTPClient http;
            http.begin(serverName);

            // Specify content-type header
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                String httpRequestData = "api_key=" + apiKeyValue 
                           + "&val1=" + val1 
                           + "&val2=" + val2
                           + "&val3=" + val3
                           + "&val4=" + val4/10.0
                           + "&val5=" + val5
                           + "&val6=" + val6/10.0
                           + "&val7=" + val7
                           + "";
            Serial.print("httpRequestData: ");
            Serial.println(httpRequestData);
            // You can comment the httpRequestData variable above
            // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
            //String httpRequestData = "api_key=YOUR_API_KEY&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";

            // Send HTTP POST request
            int httpResponseCode = http.POST(httpRequestData);
            if (httpResponseCode>0) {
              Serial.print("HTTP Response code: ");
              Serial.println(httpResponseCode);
            }
            else {
              Serial.print("Error code: ");
              Serial.println(httpResponseCode);
            }
            // Free resources
            http.end();

            break;

          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  // Convert temperature to Fahrenheit
  /*Serial.print("Temperature = ");
  Serial.print(1.8 * bme.readTemperature() + 32);
  Serial.println(" *F");*/
  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}

 
byte nitrogen(){
  uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);

  mod.write(nitro, sizeof(nitro));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
    }
  }
  Serial.println();
  return values[4];
}
 
byte phosphorous(){

  uint32_t startTime = 0;
  uint8_t  byteCount = 0;

  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  mod.write(phos, sizeof(phos));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
      
    }
  }
  Serial.println();
  return values[4];

}
 
byte potassium(){
  uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  mod.write(pota, sizeof(pota));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
    }
  }
  Serial.println();
  return values[4];
}

byte ph(){
   uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(soil_ph, sizeof(soil_ph));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
    }
  }
  Serial.println();
  return values[4];
}

byte moisture(){
   uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(soil_moist, sizeof(soil_moist));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
    }
  }
  Serial.println();
  return values[4];
}

byte temp(){
  uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(soil_temp, sizeof(soil_temp));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
    }
  }
  Serial.println();
  return values[4];
}

byte EV(){
   uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(soil_cond, sizeof(soil_cond));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while ( millis() - startTime <= TIMEOUT ) {
    if (mod.available() && byteCount<sizeof(values) ) {
      values[byteCount++] = mod.read();
      printHexByte(values[byteCount-1]);
    }
  }
  Serial.println();
  return values[4];
}

void printHexByte(byte b)
{
  Serial.print((b >> 4) & 0xF, HEX);
  Serial.print(b & 0xF, HEX);
  Serial.print(' ');
  lcd.clear();
}

