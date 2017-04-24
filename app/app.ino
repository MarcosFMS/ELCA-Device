// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Please use an Arduino IDE 1.6.8 or greater

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

#include "config.h"

static bool messagePending = false;
static bool messageSending = true;

static char *connectionString = "HostName=elca-iot.azure-devices.net;DeviceId=elca-main-device;SharedAccessKey=HZUhl6GN3+DfuYV3g7jwwG58KPyF0aGz/9hxMGX2EYU=";
static char *ssid = "Marcos Network";
static char *pass = "ace6d52f";

static int interval = INTERVAL;

void blinkLED()
{
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void initTime()
{
    time_t epochTime;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0)
        {
            LogInfo("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            LogInfo("Fetched NTP epoch time is: %lu", epochTime);
            break;
        }
    }
}

int testaWIFI(){
  WiFi.begin(ssid, pass);//Define o ssid e senha da rede
  int c = 0;
  Serial.println(String("Conectando na rede wifi (")+ssid+","+pass+")");  
  while ( c < 50 ) {//Espera 10 segundos até conectar-se a rede
    Serial.println(String("Tentativa ")+(c+1)+"/50");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("IP Local: "); 
      Serial.println(WiFi.localIP());
      return(1); 
    } //Verifica se esta conectado
    delay(500); 
    c++;
  }
  Serial.println("Falha de conexão (Time Out)");
  return 0;
}

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
void setup()
{
    delay(2000);
    initSerial();
    if(testaWIFI()){
      
      initTime();
      initIoThubClient();
  
      iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
      if (iotHubClientHandle == NULL)
      {
          LogInfo("Failed on IoTHubClient_CreateFromConnectionString");
          while (1);
      }
  
      IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
      IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
      IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);
    }
}

static int messageCount = 1;
void loop()
{
    if (!messagePending && messageSending)
    {
        char messagePayload[MESSAGE_MAX_LEN];
        bool temperatureAlert = readMessage(messageCount, messagePayload);
        sendMessage(iotHubClientHandle, messagePayload, temperatureAlert);
        messageCount++;
        delay(interval);
    }
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    delay(10);
}
