#include "WiFi.h"
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>

#define PIN_TO_SENSOR   34 
#define LED_BLUE   25 
#define LED_GREEN   26 
#define LED_RED   27 

const char* ssid = "DOMOTICACASA";
const char* password = "33333333";

int domoticzDevice = 1767;
char domoticzuser[20];
char domoticzpass[20];
char domoticzserver[40];


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int sleepdelay = 500;
int longsleepdelay = 25000;

time_t currenttime;
struct tm *now_tm;
int currentminute;
int currenthour;
int currentday;
int previousminute;
int previoushour;
int previousday;


int currentStatus=0;
int previousStatus=0;
int literslastminute=0;
int literslasthour=0;
int literslastday=0;
int connectedDomoticz=0;
String  inputMessage1 = "12345";
String  inputMessage2 = "lgarrido";
String  inputMessage3 = "5523434a";
String  inputMessage4 = "http://192.168.2.16:8080";

HTTPClient httpClient;

AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "domoticzID";
const char* PARAM_INPUT_2 = "domoticzUSER";
const char* PARAM_INPUT_3 = "domoticzPASS";
const char* PARAM_INPUT_4 = "domoticzSERVER";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Water Meter</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>Current Values</h2> 
  <p>%CURRENTMETERSTATUS%</p>
  <p>%CURRENTDOMOTICZSTATUS%</p>
  <p>Liters last minute: %LITERSMINUTE%</p>
  <p>Liters current hour: %LITERSHOUR%</p>
  <p>Liters current day: %LITERSDAY%</p>
  <h2>Domoticz Configuration</h2>
  <form action="/get">
    Device ID: <input type="number" name="domoticzID" value="%DOMOTICZID%" required><br>
    User:&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp <input type="text" name="domoticzUSER" value="%domoticzUSER%" required><br>
    Password: &nbsp<input type="password" name="domoticzPASS" value="%domoticzPASS%" required><br>
    Server:&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp <input type="url" name="domoticzSERVER" value="%domoticzSERVER%" required><br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String processor(const String& var){


  if(var == "CURRENTMETERSTATUS")
  {
    if (currentStatus==1)
    { 
       return String("In contact with Water meter magnet!");
    }
    else
     { 
       return String("No contact with Water meter magnet!");
    }    
  }
  else if(var == "CURRENTDOMOTICZSTATUS"){
    if (connectedDomoticz==1)
    { 
       return String("Connected to Domoticz!");
    }
    else
     { 
       return String("Not connected to Domoticz!");
    } 
  }
  else if(var == "LITERSMINUTE"){
    return String(literslastminute);
  }
  else if(var == "LITERSHOUR"){
    return String(literslasthour);
  }
  else if(var == "LITERSDAY"){
    return String(literslastday);
  }
  else if(var == "DOMOTICZID"){
    return String(domoticzDevice);
  }
  else if(var == "domoticzUSER"){
    return String(domoticzuser);
  }
  else if(var == "domoticzPASS"){
    return String(domoticzpass);
  }
  else if(var == "domoticzSERVER"){
    return String(domoticzserver);
  }
  return String();
}

void initWiFi() 
{
  int i=0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(5000);
    if (i>10)
    {
      Serial.println("Not possible to start Wifi!");
      return; 
    }
    i++;
  }
  Serial.println(WiFi.localIP());
}

void initTime()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void initDomoticz()
{
  char domoticzcommand[100];
  domoticzcommand[0]='\0';

  strcat(domoticzcommand,domoticzserver);
  strcat(domoticzcommand,"/json.htm?type=settings");

  Serial.println("Connecting to Domoticz...");
  Serial.printf("Domoticz Server Executing command: %s\n", domoticzcommand);
  httpClient.begin(domoticzcommand);
  httpClient.setAuthorization(domoticzuser, domoticzpass);

  int statusCode = httpClient.GET();
  if (statusCode > 0) 
  {
    if(statusCode == HTTP_CODE_OK) 
    {
      Serial.println("Domoticz Server OK!");
      String payload = httpClient.getString();
      Serial.println(payload);
      connectedDomoticz=1;
    }
    else 
    {
      Serial.printf("Domoticz Server HTTP KO! Error code: %d", statusCode);
      String payload = httpClient.getString();
      Serial.println(payload);
      connectedDomoticz=0;
    }
  }
  else 
  {
    Serial.printf("Error occurred while chechink Domoticz server:  %s\n", httpClient.errorToString(statusCode).c_str());
    connectedDomoticz=0;
  }

  httpClient.end();
}

void initWebServer()
{
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Receive an HTTP GET request at <ESP_IP>/get?domoticzID=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET threshold_input value on <ESP_IP>/get?domoticzID=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        if (request->hasParam(PARAM_INPUT_3)) {
           inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
           if (request->hasParam(PARAM_INPUT_4)) {
               inputMessage4 = request->getParam(PARAM_INPUT_4)->value();
           }
        }
      }
    }
    domoticzDevice = atoi(inputMessage1.c_str());
    Serial.println("New Domoticz Device:");
    Serial.println(domoticzDevice);
    strcpy(domoticzuser,inputMessage2.c_str());
    Serial.println("New Domoticz User:");
    Serial.println(domoticzuser);
    strcpy(domoticzpass,inputMessage3.c_str());
    Serial.println("New Domoticz Pass:");
    Serial.println(domoticzpass);
    strcpy(domoticzserver,inputMessage4.c_str());
    Serial.println("New Domoticz Server:");
    Serial.println(domoticzserver);
    request->send(200, "text/html", "Configuration updated!.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();

}
void setup()
{

  pinMode(PIN_TO_SENSOR,INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  analogWrite(LED_BLUE, 0);
  analogWrite(LED_RED, 255);
  analogWrite(LED_GREEN, 0); 

  domoticzuser[0]='\0';
  strcat(domoticzuser,inputMessage2.c_str());
  domoticzpass[0]='\0';
  strcat(domoticzpass,inputMessage3.c_str());
  domoticzserver[0]='\0';
  strcat(domoticzserver,inputMessage4.c_str());

  Serial.begin(9600);

  initWiFi();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to Wifi !");
  }


  initTime();

  initWebServer();

  initDomoticz();

}

int updateDomoticzSensor(int sensor, int value )
{
  char domoticzcommand[100];
  domoticzcommand[0]='\0';
  char sensorstring[10];
  char valuestring[10];
  sprintf(sensorstring, "%d", sensor);
  sprintf(valuestring, "%d", value);

  strcat(domoticzcommand,domoticzserver);
  strcat(domoticzcommand,"/json.htm?type=command&param=udevice&idx=");
  strcat(domoticzcommand,sensorstring);
  strcat(domoticzcommand,"&svalue=");
  strcat(domoticzcommand,valuestring);

  Serial.printf("Domoticz Server Executing command: %s\n", domoticzcommand);
  httpClient.begin(domoticzcommand);
  httpClient.setAuthorization(domoticzuser, domoticzpass);

  int statusCode = httpClient.GET();
  if (statusCode > 0) 
  {
    if(statusCode == HTTP_CODE_OK) 
    {
      Serial.printf ("New liters added: %d. ",value);
      Serial.println("Domoticz Server OK!");
      connectedDomoticz=1;
    }
    else 
    {
      Serial.printf("Domoticz Server HTTP KO! Error code: %d", statusCode);
      String payload = httpClient.getString();
      Serial.println(payload);
      connectedDomoticz=0;
    }
  }
  else {
    Serial.printf("Error occurred while cheching Domoticz server:  %s\n", httpClient.errorToString(statusCode).c_str());
    connectedDomoticz=0;
  }


  httpClient.end();

  return statusCode;
}

void loop()
{
  int sensorValue = analogRead(PIN_TO_SENSOR); 

  currenttime = time(NULL);
  now_tm = localtime(&currenttime);

  currentminute = now_tm->tm_min;
  currenthour = now_tm->tm_hour;
  currentday = now_tm->tm_mday;

  if(sensorValue < 10)
  { 
    currentStatus=1;
  }  
  else
  {
    currentStatus=0;
  } 

  if ((currentStatus!=previousStatus) && (currentStatus==1) )
  {
    literslastminute=literslastminute+1;
    literslasthour=literslasthour+1;
    literslastday=literslastday+1;
  }

  if (currentminute!=previousminute)
  {
    if (literslastminute>0)
    {
      updateDomoticzSensor(domoticzDevice,literslastminute);
    }
    if (currenthour!=previoushour)
    {
       literslasthour=0;
       previoushour=currenthour;
    }

    if (currentday!=previousday)
    {
       literslastday=0;
       previousday=currentday;
    }

    previousminute=currentminute;

    if (connectedDomoticz==1)
    {
      literslastminute=0;
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        connectedDomoticz=0;
    }
  }
  if (connectedDomoticz==0)
  {
    analogWrite(LED_BLUE, 0);
    analogWrite(LED_RED, 255);
    analogWrite(LED_GREEN, 0);       
  }
  else if (currentStatus==1)
  {
    analogWrite(LED_BLUE, 255);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
  }
  else if (connectedDomoticz==1)
  {
    analogWrite(LED_BLUE, 0);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 255);
  }
  else
  {
    analogWrite(LED_BLUE, 0);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);    
  }

  if (connectedDomoticz==0)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Wifi connection lost!");
      WiFi.disconnect();
      initWiFi();
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("Connected to Wifi, again!");
        initDomoticz();
      }
    }
    else
    {
      Serial.println("Wifi if fine, but no connectivity with Domoticz");
      initDomoticz();
    }
    delay(longsleepdelay);
  }

  previousStatus=currentStatus;
  delay(sleepdelay);
}