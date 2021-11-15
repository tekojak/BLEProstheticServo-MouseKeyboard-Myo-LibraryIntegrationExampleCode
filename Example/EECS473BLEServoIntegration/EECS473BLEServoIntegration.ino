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
const int buttonModePin = 15;  
const int ledGreenPin =  27;  
const int ledBluePin = 26;
// Globals that will control button interrupts and corresponding for-loop functionality
volatile byte modeState = 0;
volatile byte modeTrigger = LOW;
// Class instantiation
myoLDAComboClass myoTest;
BLEClass Test;
ArmServo testServo1;
ArmServo testServo2;
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
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);
  // Initialize the pushbutton pins as interrupt inputs:
  pinMode(buttonModePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonModePin), modeISR, HIGH);
  /*Init servo 1 */
  Serial.println("Initializing Servo 1");
  testServo1.z_servo_pin = 14;  
  testServo1.z_servo_micro_open = 2500;
  testServo1.z_servo_micro_closed = 500;
  testServo1.z_servo_micro_max = 2500;
  testServo1.z_servo_micro_min = 500;
  testServo1.z_MOSFET_pin = 32;
  testServo1.z_servo_speed = 210;
  testServo1.servoInit();
  Serial.println("Initializing Successful!");
  /*Init servo 2 */
  Serial.println("Initializing Servo 2");
  testServo2.z_servo_pin = 5;
  testServo2.z_servo_micro_open = 2500;
  testServo2.z_servo_micro_closed = 500;
  testServo2.z_servo_micro_max = 2500;
  testServo2.z_servo_micro_min = 500;
  testServo2.z_MOSFET_pin = 33;
  testServo2.z_servo_speed = 210;
  testServo2.servoInit();
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
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledBluePin, LOW);
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
      digitalWrite(ledGreenPin, LOW);
      digitalWrite(ledBluePin, HIGH);
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
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledBluePin, HIGH);
      output = myoTest.debounceMyoPredictions();
      myoTest.lockState(output);
      if(output == 1)
      {
        testServo1.servoOpen();
        testServo2.servoOpen();
      }
      else if(output == 2)
      {
        testServo1.servoOpen();
        testServo2.servoClosed();
      }
      else if(output == 3)
      {
        testServo1.servoClosed();
        testServo2.servoOpen();
      }
      else if(output == 4)
      {
        testServo1.servoClosed();
        testServo2.servoClosed();
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
  modeTrigger = HIGH;
}
