#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include "html.h"

#define motor_fl_pin 12
#define motor_fr_pin 13
#define motor_rl_pin 14
#define motor_rr_pin 15

const char* ssid = "HONOR 80 GT";
const char* password = "12345678";
uint8_t mpuIntStatus; // MPU6050的中断状态
uint8_t fifoBuffer[64]; // FIFO缓冲区，用于存储传感器数据
MPU6050 mpu;
ESP8266WebServer server(80);
volatile bool mpuInterrupt = false; // 定义一个标志位来指示中断是否发生

float gear = 0;

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handlePosition() {
  // 获取GET请求中的参数
  String canvasId = server.arg("canvasId");
  String xValue = server.arg("x");
  String yValue = server.arg("y");

  // // 打印位置信息
  // Serial.print("Joystick ");
  // Serial.print(canvasId);
  // Serial.print(" position - X: ");
  // Serial.print(xValue);
  // Serial.print(", Y: ");
  // Serial.println(yValue);


  gear = yValue.toInt();
  
  // 发送响应
  server.send(200, "text/plain", "Position received");
}

// IRAM_ATTR void dmpDataReady() {
//     mpuInterrupt = true;
// }
//*********Use FreeRTOS***********
float ypr_init[3];
uint8_t devStatus;
void mpu6050_init()
{
  mpu.initialize();
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  devStatus = mpu.dmpInitialize();

  if (devStatus == 0)
  {
    // 启用DMP
    mpu.setDMPEnabled(true);

    // // 配置中断并启用中断检测
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);                                                 
    // mpuIntStatus = mpu.getIntStatus();

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

int32_t init_count = 0;
void mpu6050dmp_Read()
{
    // 获取传感器数据

    // 获取DMP数据
    uint16_t fifoCount = mpu.getFIFOCount();
    if (fifoCount >= 1024)
    {
      // 清除FIFO
      mpu.resetFIFO();
      Serial.println(F("FIFO overflow!"));
    }
    else
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

      if(init_count < 2000)
      {
        init_count +=1;
        memcpy(ypr_init,ypr,sizeof(ypr));
      }

      // 输出欧拉角
      Serial.print("Yaw, Pitch, Roll: ");
      Serial.print((ypr[0] - ypr_init[0]) * 180 / M_PI);
      Serial.print(", ");
      Serial.print((ypr[1] - ypr_init[1]) * 180 / M_PI);
      Serial.print(", ");
      Serial.println((ypr[2] - ypr_init[2]) * 180 / M_PI);

    }
    

    delay(5);

  }

void setup() {
  // put your setup code here, to run once:
  analogWrite(motor_fl_pin, 0);
  analogWrite(motor_fr_pin, 0);
  analogWrite(motor_rr_pin, 0);
  analogWrite(motor_rl_pin, 0);
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  WiFi.begin(ssid, password);
  
  // pinMode(motor_fl_pin, )
  // pinMode(INTERRUPT_PIN, INPUT);

  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(".");

  // }
  Serial.println("");
  Serial.print("Connected sucsess!['-'],ip adress:");
  Serial.println(WiFi.localIP());

  
  // 设置路由
  server.on("/", HTTP_GET, handleRoot);
  server.on("/position", HTTP_GET, handlePosition);

  server.begin();
  Serial.println("HTTP server started");
  
  mpu6050_init();

  gear = 100;
  analogWrite(motor_fl_pin, 0);
  analogWrite(motor_fr_pin, 0);
  analogWrite(motor_rr_pin, 0);
  analogWrite(motor_rl_pin, 0);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // server.handleClient();
  if(devStatus == 0)
  {
  mpu6050dmp_Read();
  }
  // Serial.println(gear);
  // analogWrite(motor_fl_pin, fabs(gear-100));
  // analogWrite(motor_fr_pin, fabs(gear-100));
  // analogWrite(motor_rr_pin, fabs(gear-100));
  // analogWrite(motor_rl_pin, fabs(gear-100));
}
