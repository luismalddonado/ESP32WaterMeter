#include "WiFi.h"
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>

#define PIN_TO_SENSOR   34 
#define LED_BLUE   25 
#define LED_GREEN   26 
#define LED_RED   27 

/* Wifi configuration*/
const char* ssid = "DOMOTICACASA";
const char* password = "33333333";

/* Domoticz configuration*/
int domoticzDevice = 1767;
const char*  domoticzuser="lgarrido";
const char*  domoticzpass="5523434a";
const char*  domoticzserver="http://192.168.2.16:8080";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int sleepdelay = 500;
int longsleepdelay = 25000;

const int MAX_VALUE= 15;

time_t currenttime;
struct tm *now_tm;
int currentminute;
int currenthour;
int currentday;
int previousminute=-1;
int previoushour=-1;
int previousday=-1;

int currentStatus=0;
int previousStatus=0;
int literslastminute=0;
int literslasthour=0;
int literslastday=0;
int connectedDomoticz=0;

HTTPClient httpClient;

AsyncWebServer server(80);

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
</body></html>)rawliteral";


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
      //String payload = httpClient.getString();
      //Serial.println(payload);
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


int updateDomoticzSensor(int value )
{
  char domoticzcommand[150];
  domoticzcommand[0]='\0';
  char sensorstring[10];
  char valuestring[10];
  sprintf(sensorstring, "%d", domoticzDevice);
  sprintf(valuestring, "%d", value);

  /* sensor update in domoticz*/
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
    Serial.printf("Error occurred while checking Domoticz server:  %s\n", httpClient.errorToString(statusCode).c_str());
    connectedDomoticz=0;
  }

  httpClient.end();

  /* Log trace to domoticz logs*/
  if (connectedDomoticz==1)
  {
    domoticzcommand[0]='\0';

    strcat(domoticzcommand,domoticzserver);
    strcat(domoticzcommand,"/json.htm?type=command&param=addlogmessage&message=WaterMeterLitersadded:");
    strcat(domoticzcommand,valuestring);
    strcat(domoticzcommand,"&level=2");

    //Serial.printf("Domoticz Server Executing command: %s\n", domoticzcommand);
    httpClient.begin(domoticzcommand);
    httpClient.setAuthorization(domoticzuser, domoticzpass);

    int statusCode2 = httpClient.GET();

    httpClient.end();
  }

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
    /* Protection to avoid wrong value if sensor get crazy*/
    if (literslastminute>MAX_VALUE)
    {
      Serial.println("No values will be uploaded!");
      literslasthour=literslasthour-literslastminute;
      literslastday=literslastday-literslastminute;
      literslastminute=0;

    }
    if (literslastminute>0)
    {
      
      updateDomoticzSensor(literslastminute);
    }
    if (currenthour!=previoushour)
    {
      /* one HTTP GET to domoticz every hour at least*/
      if (literslastminute==0)
      {
        updateDomoticzSensor(0);
      }

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
    /* Long sleep. There ia problem */
    delay(longsleepdelay);
  }

  previousStatus=currentStatus;

  delay(sleepdelay);

}
