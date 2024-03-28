#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include "html.h"

#define INTERRUPT_PIN 14  // 使用中断引脚来接收数据

const char* ssid = "403-left";
const char* password = "qwertyuiop";
uint8_t mpuIntStatus; // MPU6050的中断状态
uint8_t fifoBuffer[64]; // FIFO缓冲区，用于存储传感器数据
MPU6050 mpu;
ESP8266WebServer server(80);
volatile bool mpuInterrupt = false; // 定义一个标志位来指示中断是否发生

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handlePosition() {
  // 获取GET请求中的参数
  String canvasId = server.arg("canvasId");
  String xValue = server.arg("x");
  String yValue = server.arg("y");

  // 打印位置信息
  Serial.print("Joystick ");
  Serial.print(canvasId);
  Serial.print(" position - X: ");
  Serial.print(xValue);
  Serial.print(", Y: ");
  Serial.println(yValue);

  // 发送响应
  server.send(200, "text/plain", "Position received");
}

IRAM_ATTR void dmpDataReady() {
    mpuInterrupt = true;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

  WiFi.begin(ssid, password);
  mpu.initialize();
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  uint8_t devStatus = mpu.dmpInitialize();
  pinMode(INTERRUPT_PIN, INPUT);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

  }
  Serial.println("");
  Serial.print("Connected sucsess!['-'],ip adress:");
  Serial.println(WiFi.localIP());

  
  // 设置路由
  server.on("/", HTTP_GET, handleRoot);
  server.on("/position", HTTP_GET, handlePosition);

  server.begin();
  Serial.println("HTTP server started");
  // 加载DMP固件
  
  if (devStatus == 0)
  {
    // 启用DMP
    mpu.setDMPEnabled(true);

    // 配置中断并启用中断检测
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);                                                 
    mpuIntStatus = mpu.getIntStatus();

    // 设置更新速率
    mpu.setRate(4); // 200Hz更新率

    Serial.println(F("DMP initialized"));
  }
  else
  {
    Serial.print(F("DMP initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  if (mpuInterrupt)
  {
    // 获取传感器数据
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // 获取DMP数据
    uint16_t fifoCount = mpu.getFIFOCount();
    if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024)
    {
      // 清除FIFO
      mpu.resetFIFO();
      Serial.println(F("FIFO overflow!"));
    }
    else if (mpuIntStatus & _BV(MPU6050_INTERRUPT_DMP_INT_BIT))
    {
      // 读取DMP数据
      while (fifoCount < mpu.dmpGetFIFOPacketSize())
      {
        fifoCount = mpu.getFIFOCount();
      }
      mpu.getFIFOBytes(fifoBuffer, mpu.dmpGetFIFOPacketSize());
      fifoCount -= mpu.dmpGetFIFOPacketSize();

      // 获取欧拉角
      Quaternion q;
      VectorFloat gravity;
      float ypr[3];
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

      // // 输出欧拉角
      // Serial.print("Yaw, Pitch, Roll: ");
      // Serial.print(ypr[0] * 180 / M_PI);
      // Serial.print(", ");
      // Serial.print(ypr[1] * 180 / M_PI);
      // Serial.print(", ");
      // Serial.println(ypr[2] * 180 / M_PI);
    }
  }
}
