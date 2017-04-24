/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>

/* Set these to your desired credentials. */
const char *ssid = "NodeMCUNetwork";
const char *password = "123456789";
String wifissid;//ssid da rede na qual o esp se conecta
String wifipassword;//senha da rede na qual o esp se conecta
const char* id_equipamento = "5";
String wifiIP;

WiFiServer server(80);
WiFiClient client;

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
int verificaEEPROM(){
  //Le os primeiros 32 caracteres armazenados na memoria eeprom
  char c;
  for (int i = 0; i < 33; ++i)
  {
    c = char(EEPROM.read(i));
    if(c == '&')
      break;
    wifissid += c;
  }
  //Le os 32 ultimos caracteres armazenados na eeprom
  for (int i = 33; i < 98; ++i)
  {
    c = char(EEPROM.read(i));
    if(c == '&')
      break;
    wifipassword += c;
  }
  if(((String)wifissid).length() > 0)
    return 1;
  return 0;
}

void defineDadosWIFI(String data)
{
  wifissid = data.substring(0, data.indexOf('&'));
  wifipassword = data.substring(data.indexOf('&')+1, data.length());
  wifissid.replace("%20", " ");
  wifipassword.replace("%20", " ");
  Serial.println(wifissid);
  Serial.println(wifipassword);
  int i;
  for(i = 0; i < wifissid.length(); i++){
    EEPROM.write(i, wifissid[i]);
  }
  EEPROM.write(i, '&');
  for(i = 0; i < wifipassword.length(); i++){
    EEPROM.write(33+i, wifipassword[i]);
  }
  EEPROM.write(33+i, '&');
  EEPROM.commit();
}

int testaWIFI(){
  WiFi.begin(wifissid.c_str(), wifipassword.c_str());//Define o ssid e senha da rede
  int c = 0;
  Serial.println(String("Conectando na rede wifi (")+wifissid+","+wifipassword+")");  
  while ( c < 50 ) {//Espera 10 segundos até conectar-se a rede
    Serial.println(String("Tentativa ")+(c+1)+"/50");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("IP Local: "); 
      Serial.println(WiFi.localIP());
      wifiIP = WiFi.localIP().toString();
      return(1); 
    } //Verifica se esta conectado
    delay(500); 
    c++;
  }
  Serial.println("Falha de conexão (Time Out)");
  return 0;
}


int testaWIFI2(){//Para realizar testes
  WiFi.begin("", "");//Define o ssid e senha da rede
  int c = 0;
  Serial.println(String("Conectando na rede wifi (")+wifissid+","+wifipassword+")");  
  while ( c < 5 ) {//Espera 10 segundos até conectar-se a rede
    Serial.println(String("Tentativa ")+(c+1)+"/5");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("IP Local: "); 
      Serial.println(WiFi.localIP());
      wifiIP = WiFi.localIP().toString();
      return(1); 
    } //Verifica se esta conectado
    delay(500); 
    c++;
  }
  Serial.println("Falha de conexão (Time Out)");
  return 0;
}

void receiveWIFIData(String request){
    String value = "";
    Serial.println(request);
    //Return response to the client
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); // do not forget this one
    client.print((String)id_equipamento);
    client.stop();
    delay(1);
    Serial.println("Client disconnected");
    Serial.println("");
    value = request.substring(request.indexOf("=")+1, request.indexOf("|"));
    defineDadosWIFI(value);
    Serial.println(value);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    testaWIFI();
}

void setup() {
	delay(1000);
  EEPROM.begin(512);
	Serial.begin(115200);
	Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if(verificaEEPROM() && testaWIFI2()){ //verifica se existe algum ssid definido
    Serial.println("Conectado a rede");
    
  }else{
  	Serial.print("Configuring access point...");
  	/* You can remove the password parameter if you want the AP to be open. */
    WiFi.mode(WIFI_AP);
    WiFi.disconnect();
  	WiFi.softAP(ssid, password);
  
  	IPAddress myIP = WiFi.softAPIP();
  	Serial.print("AP IP address: ");
  	Serial.println(myIP);
    server.begin();
  	Serial.println("HTTP server started");
  }
}

void loop() {// Verifica se algum cliente conectou
    client = server.available();
    if (!client) {
    return;
    }
    
    // Espera ate o cliente enviar algum dado
    Serial.println("Novo cliente");
    
    // Le o request do cliente
    String request = client.readStringUntil('\r');
    String function = request.substring(request.indexOf("?")+1, request.indexOf("="));
    if(function.equals("wifidata")){
      receiveWIFIData(request);
    }
    /*
    if(function.equals("turndevice")){
      turnPower(request, client);
    }
    if(function.equals("wifiip")){
      sendWifiIptoClient(client);
    }*/
}
