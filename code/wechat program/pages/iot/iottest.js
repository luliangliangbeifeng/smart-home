// 引入包
var iot = require('../../utils/alibabacloud-iot-device-sdk.min');

// 定义云端创建的设备三元组信息，并使用协议声明，使用 "protocol": 'alis://'
const sdk_device = {
  "productKey": "a1L8S248v7Q",
  "deviceName": "test",
  "deviceSecret": "xEiuaCRcJiXK59586Sd9bLGOj7s26sDG",
  "protocol": 'wxs://',
};
var device = iot.device(sdk_device);
var subscribeTopic = `/sys/${sdk_device.productKey}/${sdk_device.deviceName}/thing/service/property/set`;
var publishTopic = `/sys/${sdk_device.productKey}/${sdk_device.deviceName}/thing/event/property/post`
var LightSwitchState = 0;
var FanSwitchState = 0;

function getPostData(id) {
  var payloadJson = {
    id: Date.now(),
    params: {},
    method: "thing.event.property.post",
    version: '1.0'
  }
  switch (id) {
    case "btnLight":
      payloadJson.params.LightSwitch = 1 ^ LightSwitchState;
      break;
    case "btnFan":
      payloadJson.params.FanSwitch = 1 ^ FanSwitchState;
      break;
  }
  return JSON.stringify(payloadJson);
}


Page({

  data: {
    img_url: "/img/door_a.png",
    password_input: "",
    key: "123456",
    login: false,
    button_clicking: false,
    iot_connect: false,
    connect_text: "未连接",

    CurrentTemperature: "--",
    Humidity: "--",
    illumination: "--",
    LightSwitch: 0,
    FanSwitch: 0,
  },

  inputPwd: function (e) {
    this.setData({
      password_input: e.detail.value
    })
  },

  confirmPwd: function () {
    var pwd = this.data.password_input;
    var that = this
    if (pwd != this.data.key) {
      wx.showToast({
        title: '密码错误',
        icon: 'none',
        duration: 2000
      })
    } else {
      wx.showToast({
        title: '验证通过',
        icon: 'success',
        duration: 2000
      })
      wx.setStorage({
        key: "password",
        data: pwd,
      })
      this.login()
    }
  },

  login: function () {
    var that = this
    wx.getStorage({
      key: 'password',
      success(res) {
        console.log(res)
        var pwd = res.data
        if (pwd == that.data.key) {
          that.setData({
            login: true
          })
          that.doConnect()
        }
      }
    })
  },

  onLoad: function () {
    this.login()
  },
  /**
   * 页面的初始数据
   */
  // data: {
  //   CurrentTemperature: "--",
  //   Humidity: "--",
  //   illumination: "--",
  //   LightSwitch: 0,
  //   FanSwitch: 0,
  // },

  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function () {
    this.doConnect()
  },

  doConnect() {
    var that=this;
    // 连接云平台
    device.on('connect', () => {
      console.log('连接成功....');

      //向指定的topic发送一个消息
      device.publish(publishTopic, JSON.stringify({
        id: Date.now(),
        version: '1.0',
        params: {
          'LightSwitch': 1,
        },
        method: 'thing.event.property.post'
      }));

      //订阅指定topic
      device.subscribe(subscribeTopic);
    });

    //接收到数据时将topic以及消息打印出来
    device.on('message', function (topic, message) {
      // console.log('收到topic', topic);
      //只处理subscribeTopic消息
      if (topic === subscribeTopic) {
        console.log('收到消息:',message.toString());
        const json = JSON.parse(message.toString());
        console.log('CurrentTemperature=', json.CurrentTemperature);
        console.log('Humidity=', json.Humidity);
        console.log('illumination=', json.illumination);
        console.log('LightSwitch=', json.LightSwitch);
        console.log('FanSwitch=', json.FanSwitch);
        if (typeof (json.LightSwitch) !== "undefined") {
          LightSwitchState = json.LightSwitch;
        }
        if (typeof (json.FanSwitch) !== "undefined") {
          FanSwitchState = json.FanSwitch;
        }
        console.log('LightSwitchState=', LightSwitchState);
        console.log('FanSwitchState=', FanSwitchState);
        that.setData(json);
      }
    });
  },


  changeStatus: function (e) {
    // //方法一：
    // var json = {};
    // if (e.target.id === "btnLight") {
    //   json.LightSwitch = 1 ^ LightSwitchState;
    // } else if (e.target.id === "btnFan") {
    //   json.FanSwitch = 1 ^ FanSwitchState;
    // }
    // //上报设备属性
    // console.log(json);
    // device.postProps(json, (res) => {
    //   console.log(res);
    // 无论发送数据是否符合物模型属性都返回code:200,故不选用
    //   if (e.target.id === "btnLight") {
    //     LightSwitchState = json.LightSwitch;
    //   } else if (e.target.id === "btnFan") {
    //     FanSwitchState = json.FanSwitch;
    //   }
    //   this.setData(json);
    //   console.log('LightSwitchState=', LightSwitchState);
    //   console.log('FanSwitchState=', FanSwitchState);
    // });

    //方法2：
    //获取发送内容
    var JSONdata = getPostData(e.target.id)
    console.log("发送消息：" + JSONdata)
    //向指定的topic发送一个消息
    device.publish(publishTopic, JSONdata);
  },
})