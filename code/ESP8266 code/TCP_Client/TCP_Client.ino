/*
  Doit tcp test demos
  version:1.0
  2016-01-06

  Copyright (c) 2015 www.doit.am All rights reserved.

  This file is part of the esp8266 core for Arduino environment.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


/*
  1, Visit "tcp.doit.am" in your brower to get an opened ip and port
  2, Change "serverIP" and "serverPort" in this sktch, compile and run in ESPduino
  3, Use a client such as TCPUDP Debugger to setup a tcp client using the ip and port
  4, Transparent transmisstion is setup between ESPduino and the tcp client.
*/

#include <ESP8266WiFi.h>

//const char* ssid      = "jimaoyubeifeng";    //change to your own ssid
//const char* password  = "12344679";//change to your own password
const char* ssid      = "nucleisys";    //change to your own ssid
const char* password  = "12345678";//change to your own password
//const char* serverIP  = "192.168.2.194";//Andrew_sun
//const char* serverIP  = "172.20.10.2";//jimaoyubeifeng
const char* serverIP  = "172.20.10.3";//jimaoyubeifeng
int serverPort = 8266;
WiFiClient client;
bool bConnected = false;
char buff[7];
int nm = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  delay(10);
  if (bConnected == false) {
    if (!client.connect(serverIP, serverPort)) {
      Serial.println("connection failed");
      delay(5000);
      return;
    }
    bConnected = true;
    Serial.println("connection ok");
  }
  else if (client.available()) 
  {
//    Serial.println("Data is coming");
    while (client.available()) {
      buff[nm++] = client.read();      
      if (nm >= 6) break;
    }
    nm = 0;
    Serial.print(buff);
 //   client.print(buff);
    client.flush();
  }
  //串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
  if (Serial.available()) {
    size_t counti = Serial.available();
    uint8_t sbuf[counti];
    Serial.readBytes(sbuf, counti);
    client.write(sbuf, counti);
  }

}
