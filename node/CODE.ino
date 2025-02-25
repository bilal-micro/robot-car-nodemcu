#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>
#define enA D8 
#define in2 D2
#define in3 D0
#define enB D7  
#define in4 D5
#define in1 D3
using namespace websockets;

const char* ssid = "test";
const char* password = "ahmed123456";
const char* websocket_server = "ws://92.113.27.167:4558/ws";
//
WebsocketsClient client;

int  xAxis, yAxis;
int motorSpeedA = 0;
int motorSpeedB = 0;

void motor(void);

void onMessageCallback(WebsocketsMessage message) {
    // Serial.print("Received: ");
    Serial.println(message.data());
    sscanf(message.data().c_str(), "%d,%d", &xAxis, &yAxis);
    // Serial.println(xAxis);
    // Serial.print(yAxis);
}

void setup() {
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");


    Serial.print("Connecting to WebSocket: ");
    Serial.println(websocket_server);


}
void checkConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected! Reconnecting...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
    }
}

void sendTestMessage() {
    if (!client.available()) {
        Serial.println("WebSocket Disconnected! Reconnecting...");
        client.connect(websocket_server);
        client.onMessage(onMessageCallback);
        delay(2000);
        return;
    }

    //client.send("test");  // Send test message
    //unsigned long startTime = millis();  // Start timer
    /*
    while (millis() - startTime < 500) {  // 10ms timeout
        if (client.available()) {
            client.poll();
            client.onMessage(onMessageCallback);
            return;  // If we get a response, exit function
        }
    }
    */
    // Timeout reached, force reconnect WebSocket
    //Serial.println("Timeout! Reconnecting WebSocket...");
    //client.close();
    //client.connect(websocket_server);
   // client.onMessage(onMessageCallback);

}


void loop() {
  checkConnection();
  sendTestMessage();
  while(client.available()){
    delay(500);
    client.onMessage(onMessageCallback);
    client.poll();
    motor();
  }
  


    // Serial.println(xAxis);
    // Serial.println(yAxis);
    

}
void motor(){
  //if(((xAxis > 470 && xAxis < 550) && (yAxis > 470 && yAxis < 550)) || (xAxis == 0 && yAxis == 0)){
  if((xAxis == 512 && yAxis == 512) || (xAxis == 0 && yAxis == 0)){
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        // Set Motor B backward
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
    }
    else if (yAxis < 512) {
        // Set Motor A backward
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        // Set Motor B backward
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        // Convert the declining Y-axis readings for going backward from 470 to 0 into 0 to 255 value for the PWM signal for increasing the motor speed
        motorSpeedA = map(yAxis, 470, 0, 0, 255);
        motorSpeedB = map(yAxis, 470, 0, 0, 255);
      }
      else if (yAxis > 512) {
        // Set Motor A forward
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        // Set Motor B forward
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        // Convert the increasing Y-axis readings for going forward from 550 to 1023 into 0 to 255 value for the PWM signal for increasing the motor speed
        motorSpeedA = map(yAxis, 550, 1023, 0, 255);
        motorSpeedB = map(yAxis, 550, 1023, 0, 255);
      }
      // If joystick stays in middle the motors are not moving
      else {
        motorSpeedA = 0;
        motorSpeedB = 0;
      }
      // X-axis used for left and right control
      if (xAxis < 470) {
        // Convert the declining X-axis readings from 470 to 0 into increasing 0 to 255 value
        int xMapped = map(xAxis, 470, 0, 0, 255);
        // Move to left - decrease left motor speed, increase right motor speed
        motorSpeedA = motorSpeedA + xMapped;
        motorSpeedB = motorSpeedB - xMapped;
        // Confine the range from 0 to 255
        if (motorSpeedA < 0) {
          motorSpeedA = 0;
        }
        if (motorSpeedB > 255) {
          motorSpeedB = 255;
        }
      }
      if (xAxis > 550) {
        // Convert the increasing X-axis readings from 550 to 1023 into 0 to 255 value
        int xMapped = map(xAxis, 550, 1023, 0, 255);
        // Move right - decrease right motor speed, increase left motor speed
        motorSpeedA = motorSpeedA - xMapped;
        motorSpeedB = motorSpeedB + xMapped;
        // Confine the range from 0 to 255
        if (motorSpeedA > 255) {
          motorSpeedA = 255;
        }
        if (motorSpeedB < 0) {
          motorSpeedB = 0;
        }
      }
      // Prevent buzzing at low speeds (Adjust according to your motors. My motors couldn't start moving if PWM value was below value of 70)
      if (motorSpeedA < 70) {
        motorSpeedA = 0;
      }
      if (motorSpeedB < 70) {
        motorSpeedB = 0;
      }
      analogWrite(enA, motorSpeedA); // Send PWM signal to motor A
      analogWrite(enB, motorSpeedB); // Send PWM signal to motor B
}