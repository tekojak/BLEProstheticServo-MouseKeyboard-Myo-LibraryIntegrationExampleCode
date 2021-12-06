#include <EECS473Servo.h>
#include <EECS473BLECombo.h>
#include <EMGLDAWrapper.h>
#include <esp_task_wdt.h>
#define WDT_TIMEOUT 20
/*
----------------------------------
-----Local Function Prototypes-----
----------------------------------
*/
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
/*
----------------------------------
-----ISR Test Function Prototypes-----
----------------------------------
*/
void modeISR();
/*
----------------------------------
-------------Globals------------
----------------------------------
*/
// myo output global
uint8_t output;
// Globals for LED and button pins
const int buttonModePin = 23;  
const int ledRed = 19;
const int ledBlue = 18;
const int ledGreen = 5;
// Globals that will control button interrupts and corresponding for-loop functionality
volatile byte modeState = 0;
volatile byte modeTrigger = LOW;
// Class instantiation
myoLDAComboClass myoTest;
BLEClass Test;
ArmServo pinchServo;
ArmServo mrpServo;
/*
----------------------------------
-------------Main Code------------
----------------------------------
*/
void setup() 
{
  /*Set serial initialization*/
  Serial.begin(115200);
  /*Initialize LED pins for Mode Status*/
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledRed,HIGH);
  digitalWrite(ledBlue,HIGH);
  digitalWrite(ledGreen,HIGH);
  /*Initialize the pushbutton pin as interrupt input:*/
  pinMode(buttonModePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonModePin), modeISR, RISING);
  /*Init servo 1 */
  Serial.println("Initializing Pinch Servo");
  pinchServo.z_servo_pin = 14;  
  pinchServo.z_servo_micro_open = 700;
  pinchServo.z_servo_micro_closed = 2100;
  pinchServo.z_servo_micro_max = 2500;
  pinchServo.z_servo_micro_min = 500;
  pinchServo.z_MOSFET_pin = 12;
  pinchServo.z_servo_speed = 210;
  pinchServo.servoInit();
  Serial.println("Pinch Servo Initialized");
  /*Init servo 2 */
  Serial.println("Initializing MRP Servo");
  mrpServo.z_servo_pin = 26;
  mrpServo.z_servo_micro_open = 2100;
  mrpServo.z_servo_micro_closed = 700;
  mrpServo.z_servo_micro_max = 2500;
  mrpServo.z_servo_micro_min = 500;
  mrpServo.z_MOSFET_pin = 27;
  mrpServo.z_servo_speed = 210;
  mrpServo.servoInit();
  Serial.println("MRP Servo Initialized");
  /*Turn on LEDs to indicate BLE-related Initializations have started*/
  digitalWrite(ledRed,LOW);
  digitalWrite(ledBlue,LOW);
  /*Turn on WDT for BLE-related Initializations*/
  esp_task_wdt_init(WDT_TIMEOUT, true);     //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                   //add current thread to WDT watch
  /*Initialize BLE Keyboard, Mouse, and MPU6050*/
  Serial.println("Initializing BLE Keyboard, Mouse, and MPU6050");
  byte status = Test.init();
  while(status != 0)
    Serial.println("Cant connect to MPU");  // Infinite loop if MPU6050 init fails.
  Serial.println("BLE Keyboard, Mouse, and MPU6050 Initialized"); 
  /*Initialize Myo Armband*/
  myoTest.setupMyo();
  esp_task_wdt_delete(NULL);  // Remove current thread to WDT watch
  esp_task_wdt_deinit();      // disable panic so that ESP32 doesnt restart
  /*Turn off  LEDs to indicate BLE-related Initializations are done*/
  digitalWrite(ledRed,HIGH);
  digitalWrite(ledBlue,HIGH);
  Serial.println("Init Sequence Complete"); 
}

void loop() 
{
  if(modeTrigger)
  {
    modeState = modeState + 1;
    if(modeState >= 3)
      modeState = 0;
    modeTrigger = LOW;
  }
  switch (modeState)
  {
    /*Keyboard State*/
    case 0:
      digitalWrite(ledRed,LOW);
      digitalWrite(ledBlue,HIGH);
      digitalWrite(ledGreen,HIGH);
      //Serial.println("Keyboard State");
      if(Test.comboKeyboard.isConnected())
      {
        myoTest.bluetoothGestureSequence(myoTest.buff);
        Serial.print(myoTest.buff[0]);
        Serial.print(myoTest.buff[1]);
        Serial.println(myoTest.buff[2]);
        output = myoTest.parse_gestures(myoTest.buff);
        Serial.print("Outputted Bluetooth Keypress: ");
        Serial.print(output,HEX);
        Serial.print("  ");
        Serial.println((char)output);
        Test.comboKeyboard.write((char)output);
      }
      else;
    break;
    /*Mouse State*/
    case 1:
      digitalWrite(ledRed,HIGH);
      digitalWrite(ledBlue,LOW);
      digitalWrite(ledGreen,HIGH);
      //Serial.println("Mouse State");
      output = myoTest.debounceMyoPredictions();
      myoTest.lockState(output);
      if(Test.comboKeyboard.isConnected())
      {
        if(output == 1)
        {
          Test.comboMouse.click(MOUSE_LEFT);
        }
        else if(output == 2)
        {
          Test.comboMouse.click(MOUSE_RIGHT);
        }
        else
        {
          Test.mouseMove();
        }
        Serial.println(output);
      }
      else;
    break;
    /*Arm State*/
    case 2:
      digitalWrite(ledRed,LOW);
      digitalWrite(ledBlue,LOW);
      digitalWrite(ledGreen,HIGH);
      //Serial.println("Arm State");      
      output = myoTest.debounceMyoPredictions();
      myoTest.lockState(output);
      if(output == 1)
      {
        pinchServo.servoOpen();
        mrpServo.servoOpen();
      }
      else if(output == 2)
      {
        pinchServo.servoOpen();
        mrpServo.servoClosed();
      }
      else if(output == 3)
      {
        pinchServo.servoClosed();
        mrpServo.servoOpen();
      }
      else if(output == 4)
      {
        pinchServo.servoClosed();
        mrpServo.servoClosed();
      }
      else;
      Serial.println(output);
      delay(100);
        
      break;
    default:
      Serial.println("Unintended State. Please Check Source Code.");
      delay(1000);
  }
}
/*
----------------------------------
-----Local Function Initializations-----
----------------------------------
*/
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) 
{
  myoTest.emgstreamer.streamData(pData, length);
}
/*
----------------------------------
-----ISR Test Function Initializations-----
----------------------------------
*/
void modeISR()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 400)
    modeTrigger = HIGH;
  last_interrupt_time = interrupt_time;
}
