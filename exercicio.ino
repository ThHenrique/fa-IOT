#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

TaskHandle_t tTaskControllMinuteTime;

SemaphoreHandle_t mutex;

char *ntpServer = "pool.ntp.org";
struct tm timeinfo;

char *ssid = "CLARO_2G85488D";
char *pwd = "0485488D";

String serverName = "http://postman-echo.com/";

bool sharedData = false;

void setup()
{
  Serial.begin(115200);

  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL)
  {
    Serial.println("Error - Mutex not created");
  }
  
  WiFi.begin(ssid, pwd);
  connectWifi();
  
  configTime(0, 0, ntpServer);  
  

  xTaskCreatePinnedToCore(
    verifyTime,   // task function
    "VerifyTime", // task name
    10000,        // Task stack size
    NULL,         // task parameters
    1,            // task priority
    &tTaskControllMinuteTime,      // task handle
    1           // task core (core 1=loop)
  );
}

void loop()
{  
  
}

// SINCRONIZAÇÃO DE DATA E HORA 
void verifyTime(void *pvParameters)
{
  while (true)
  {
    xSemaphoreTake(mutex, portMAX_DELAY);

    // SINCRONIZAÇÃO DE DATA E HORA
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Acesso ao ntp falhou");
    }

    // TRANSMISSÃO DE DADOS A CADA 5 MIN
    if (timeinfo.tm_min % 5 == 0) {
      if (sharedData == false) {
        shareData();
      }     
    } else {
      sharedData = false;
    }  
    xSemaphoreGive(mutex); 
  }    
}

// CONEXÃO COM WIFI
void connectWifi()
{
  Serial.println("Conectando...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.print("Conectado ao WiFi com o IP: ");
  Serial.println(WiFi.localIP());
}

// TRANSMISSÃO DE INFORMAÇÕES
void shareData()
{
  if (WiFi.status() == WL_CONNECTED)
  {   
    WiFiClient client;
    HTTPClient http_post;
    
    String url = serverName + "post";
    http_post.begin(client, url);
    
    // setando headers config
    http_post.addHeader("Content-Type", "application/json"); // permitindo envio de json
    String data = "{\"MESSAGE\": \"FAZ O PIX MEU AMIGO\", \"USERNAME\": \"THIAGO HENRIQUE FERREIRA\", \"DESCRIPTION\": \"O MAIS BONITO\"}";

    int httpCode = http_post.POST(data);
    if (httpCode > 0)
    {
      Serial.println(httpCode);
      String payload = http_post.getString();
      Serial.print("Response of server: ");
      Serial.println(payload);

      sharedData = true;
    }
    else
    {
      sharedData = false;
      Serial.println("Http error");
    }
  }
  else
  {
    Serial.println("WIFI connection not established");
    connectWifi();
  }    
}
