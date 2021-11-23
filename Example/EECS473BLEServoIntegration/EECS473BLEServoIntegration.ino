#include <EECS473Servo.h>
#include <EECS473BLECombo.h>
#include <EMGLDAWrapper.h>
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
  // Set serial initialization
  Serial.begin(115200);
  Serial.println("Starting work!"); // Debug Code
  // Initialize BLE Keyboard, Mouse, and MPU6050
  Serial.println("Initializing BLE Keyboard, Mouse, and MPU6050");
  byte status = Test.init();
  while(status != 0)
    Serial.println("Cant connect to MPU");
  Serial.println("Initializing Successful!"); 
  // Initialize LED pins for Mode Status
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledRed,HIGH);
  digitalWrite(ledBlue,HIGH);
  digitalWrite(ledGreen,HIGH);
  // Initialize the pushbutton pins as interrupt inputs:
  pinMode(buttonModePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonModePin), modeISR, RISING);
  /*Init servo 1 */
  Serial.println("Initializing Servo 1");
  pinchServo.z_servo_pin = 14;  
  pinchServo.z_servo_micro_open = 700;
  pinchServo.z_servo_micro_closed = 2100;
  pinchServo.z_servo_micro_max = 2500;
  pinchServo.z_servo_micro_min = 500;
  pinchServo.z_MOSFET_pin = 12;
  pinchServo.z_servo_speed = 210;
  pinchServo.servoInit();
  Serial.println("Initializing Successful!");
  /*Init servo 2 */
  Serial.println("Initializing Servo 2");
  mrpServo.z_servo_pin = 26;
  mrpServo.z_servo_micro_open = 2100;
  mrpServo.z_servo_micro_closed = 700;
  mrpServo.z_servo_micro_max = 2500;
  mrpServo.z_servo_micro_min = 500;
  mrpServo.z_MOSFET_pin = 27;
  mrpServo.z_servo_speed = 210;
  mrpServo.servoInit();
  Serial.println("Initializing Successful!");
  myoTest.setupMyo();
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
      Serial.println("Keyboard State");
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
    break;
    /*Mouse State*/
    case 1:
      digitalWrite(ledRed,HIGH);
      digitalWrite(ledBlue,LOW);
      digitalWrite(ledGreen,HIGH);
      Serial.println("Mouse State");
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
      digitalWrite(ledRed,HIGH);
      digitalWrite(ledBlue,HIGH);
      digitalWrite(ledGreen,LOW);
      Serial.println("Arm State");      
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
