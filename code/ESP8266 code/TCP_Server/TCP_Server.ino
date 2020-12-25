#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

//#include <aliyun_mqtt.h>
int LightState = 0, FanState = 0;

//初始化现态
int last0 = 0;

int Buffer[8] = {0};

char Receive_buf[6] = {0};
//上云时间现态
unsigned long lastMqttConnectMs = 0;
unsigned long lastPostSendMs = 0;
#define MAX_SRV_CLIENTS 3

//你的wifi
//#define WIFI_SSID "jimaoyubeifeng"
//#define WIFI_PASSWD "12344679"
#define WIFI_SSID "nucleisys"
#define WIFI_PASSWD "12345678"


//----------------------------------------------------------------------------------------//
//-----------------------------------配置信息开始------------------------------------------//
//----------------------------------------------------------------------------------------//
///* 设备的三元组信息*/PRODUCT_SECRET
#define PRODUCT_KEY "a1L8S248v7Q"//设备所属产品的ProductKey
#define DEVICE_NAME "esp8266_test"//设备名称DeviceName
#define DEVICE_SECRET "amT4EWn0TJQOp9NUJQrfmZ0iSGI0xTQ0"//设备的DeviceSecret

/*区域ID*/
#define REGION_ID "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT 1883

//MQTT用户名（User Name）
#define MQTT_USRNAME DEVICE_NAME "&" PRODUCT_KEY

//MQTT客户端标识符 (ClientId)
#define CLIENT_ID "esp8266_test|securemode=3,timestamp=1592115424558,signmethod=hmacmd5|"

//MQTT密码（Password）
#define MQTT_PASSWD "C2F9FD9571F8C6B86CD53BB721527994"

//订阅主题(云端下发设置到设备)
#define ALINK_TOPIC_PROP_SET "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"

//发布主题(设备上传属性数据到云端)
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

////订阅和发布时所需的主题
#define ALINK_BODY_FORMAT "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"
#define ALINK_METHOD_PROP_POST "thing.event.property.post"

WiFiServer server(8266);
WiFiClient serverClients[MAX_SRV_CLIENTS];

//创建WiFiClient实例
WiFiClient espClient;

//创建MqttClient实例
PubSubClient mqttClient(espClient);


//连接Wifi
void initWifi(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi does not connect, try again ...");
    delay(500);
  }

  Serial.println("Wifi is connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//监听云端下发指令并处理
void callback(char *topic, byte *payload, unsigned int length)
{

  //    Serial.println();
  //    Serial.println();
  //    Serial.print("Message arrived [");
  //    Serial.print(topic);
  //    Serial.print("] ");
  //    Serial.println();
  //云端下发属性设置topic = "[/sys/a10PnXZBgsl/7L9EUbLv113EkYkF7fpD/thing/service/property/set]"

  payload[length] = '\0';
//  Serial.println((char *)payload);
  //const char *payload = "{"method":"thing.service.property.set","id":"282860794","params":{"LightSwitch":1},"version":"1.0.0"}"


  if (strcmp(topic, ALINK_TOPIC_PROP_SET) == 0)
  {
    //json解析payload
    //    StaticJsonBuffer<400> jsonBuffer;
    //    JsonObject &root = jsonBuffer.parseObject(payload);
    //判断json解析是否成功
    //    if (!root.success())
    //    {
    //      Serial.println("parseObject() failed");
    //      return;
    //    }
    //    if (root["params"]["LightSwitch"].success()) {
    //      LightState = root["params"]["LightSwitch"];
    //      Serial.print("Light=");
    //      Serial.println(LightState);
    //    }
    //    if (root["params"]["FanSwitch"].success()) {
    //      FanState = root["params"]["FanSwitch"];
    //      Serial.print("Fan=");
    //      Serial.println(FanState);
    //    }

    DynamicJsonDocument root(1024); //堆区
    DeserializationError error = deserializeJson(root, payload);
    if (error)
    {
      Serial.print("deserializeMsgPack() failed: ");
      //      Serial.println(error.c_str());
      return;
    }

    if (!root["LightSwitch"].isNull())
    {
      LightState = root["LightSwitch"];
    }
    if (!root["FanSwitch"].isNull()){
      FanState = root["FanSwitch"];
    }
    Receive_buf[0] = 0xaa;
    Receive_buf[1] = 0xbb;
    Receive_buf[2] = 0xcc;
    if (LightState == 1) {
      Receive_buf[3] = 0x01;
    } else {
      Receive_buf[3] = 0x02;
    }
    if (FanState == 1) {
      Receive_buf[4] = 0x01;
    } else {
      Receive_buf[4] = 0x02;
    }
    Serial.print(Receive_buf);
//    mqttIntervalPost();
  }
}

//MQTT客户端重新连接并返回连接状态
boolean reconnect()
{
  if (mqttClient.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD)) //用指定的客户端ID、用户名和密码连接客户端,如果连接成功
  {
    Serial.println("MQTT Connected!"); //MQTT连接成功
    //mqttClient.publish("outTopic", "hello world"); //发布hello world
    mqttIntervalPost();
    mqttClient.subscribe(ALINK_TOPIC_PROP_SET); //并重新订阅主题
  }
  else //连接失败
  {
    Serial.print(millis());
    Serial.print(" MQTT Connect err:");        //输出MQTT连接错误
    Serial.println(mqttClient.state());        //输出MQTT连接状态
    Serial.println(" try again in 2 seconds"); //输出2秒后重新连接
  }
  return mqttClient.connected(); //返回True/False,是否已连接到服务器
}


////连接Mqtt订阅属性设置Topic
//void mqttCheckConnect()
//{
//  bool connected = connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);
//
//  if (connected)
//  {
//    Serial.println("MQTT connect succeed!");
//
//    //订阅属性设置Topic
//    mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
//    Serial.println("subscribe done");
//  }
//}

// 上报属性Topic数据
void mqttIntervalPost()
{
  char param[256];
  char jsonBuf[512];


  sprintf(param, "{\"Humidity\":%d,\"CurrentTemperature\":%d,\"illumination\":%d,\"LightSwitch\":%d,\"FanSwitch\":%d}", Buffer[2], Buffer[3], Buffer[4], Buffer[5], Buffer[6]);
  //  sprintf(param, "{\"Humidity\":%d,\"CurrentTemperature\":%d,\"illumination\":%d,\"LightSwitch\":%d,\"FanSwitch\":%d}", Buffer[2], Buffer[3], Buffer[4], LightState, FanState);
  sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param);
  //    jsonBuf = "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":"\{\"S_D0\":%d}\"}"

// Serial.println(jsonBuf);
  lastPostSendMs = millis();
  mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
}


void setup()
{
  Serial.begin(115200);
  initWifi(WIFI_SSID, WIFI_PASSWD);

  //启动UART传输和服务器
  server.begin();//启动TCP server
  server.setNoDelay(true);//关闭延时发送功能

  Serial.println();
  Serial.println();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT); //可变
  mqttClient.setCallback(callback);             //回调，监听云端下发指令，当ESP8266收到订阅Topic后调用callback函数
  lastMqttConnectMs = 0;
}


void loop()
{
  uint8_t i, k;
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (!serverClients[i ] || !serverClients[i ].connected()) {
        if (serverClients[i ]) serverClients[i ].stop();//未联接,就释放
        serverClients[i ] = server.available();//分配新的
        continue;
      }

    }
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i ] && serverClients[i ].connected()) {
      if (serverClients[i ].available()) {
        while (serverClients[i ].available())
          Serial.write(serverClients[i ].read());
      }
    }
  }
  if (Serial.available() > 0) {
    //    size_t len = Serial.available();
    uint8_t sbuf[7];
    Serial.readBytes(sbuf, 7);
    if (sbuf[0] == 0xAA && sbuf[1] == 0xFF) {
      Buffer[2] = sbuf[2];
      Buffer[3] = sbuf[3];
      Buffer[4] = sbuf[4];
      Buffer[5] = sbuf[5];
      Buffer[6] = sbuf[6];
    }
    mqttIntervalPost();
    //    push UART data to all connected telnet clients
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i ] && serverClients[i ].connected()) {
        serverClients[i ].write(sbuf, 7);  //向所有客户端发送数据
      }
    }
  }



  //  if (WiFi.status() == STATION_GOT_IP) //获取到IP
  //  {
  if (!mqttClient.connected()) //如果未连接到MQTT服务器，2s后重连
  {
    if (millis() - lastMqttConnectMs > 2000) //距上次尝试连接MQTT服务器时间大于2s
    {
      lastMqttConnectMs = millis();
      //尝试重新连接
      if (reconnect()) //如果连接上MQTT服务器
      {
        lastMqttConnectMs = 0; //归0
      }
    }
  }
  else //如果连接到MQTT服务器
  {
    //            if(millis()-lastPostSendMs)
    mqttClient.loop(); //应定期调用此方法以允许客户端处理传入消息并维护其与服务器的连接
  }
  //  }

}
