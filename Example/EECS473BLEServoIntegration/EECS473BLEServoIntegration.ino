#include <EECS473Servo.h>
#include <EECS473BLECombo.h>
/*
----------------------------------
-----ISR Test Function Prototypes-----
----------------------------------
*/
void modeISR();
void clickRightISR();
void clickLeftISR();
/*
----------------------------------
-------------Globals------------
----------------------------------
*/
// Globals for LED and button pins
const int buttonModePin = 15;  
const int buttonClickRightPin = 2; 
const int buttonClickLeftPin = 4;
const int ledGreenPin =  27;  
const int ledBluePin = 26;
// Globals that will control button interrupts and corresponding for-loop functionality
volatile byte modeState = 0;
volatile byte modeTrigger = LOW;
volatile byte clickRightTrigger = LOW; 
volatile byte clickLeftTrigger = LOW; 
// Class instantiation
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
  pinMode(buttonClickRightPin,INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonClickRightPin), clickRightISR, HIGH);
  pinMode(buttonClickLeftPin,INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonClickLeftPin), clickLeftISR, HIGH);
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
    case 0:
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledBluePin, LOW);
      if(Test.comboKeyboard.isConnected())
      {
        Serial.println("Sending 'Hello world'");
        Test.comboKeyboard.println("Hello World");
        Serial.println("Sending Enter key...");
        Test.comboKeyboard.write(KEY_RETURN);
        delay(1000);
      }
      else
      {
        Serial.println("BLE Keyboard/Mouse not connected. Waiting 2 seconds...");
        delay(2000);
      }
    break;
    case 1:
      digitalWrite(ledGreenPin, LOW);
      digitalWrite(ledBluePin, HIGH);
      if(Test.comboKeyboard.isConnected())
      {
        Test.mouseMove();
        if(clickRightTrigger == HIGH)
        {
          Test.comboMouse.click(MOUSE_RIGHT);
          clickRightTrigger = LOW;
        }
        else if(clickLeftTrigger == HIGH)
        {
          Test.comboMouse.click(MOUSE_LEFT);
          clickLeftTrigger = LOW;
        }
        delay(100);
      }
      else
      {
        Serial.println("BLE Keyboard/Mouse not connected. Waiting 2 seconds...");
        delay(2000);
      }
    break;
    case 2:
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledBluePin, HIGH);
      /* Debug Code Start*/
      Serial.print("Move Servo 1 to closed: ");
      Serial.println(testServo1.z_servo_micro_current);  
      /* Debug Code End */
      testServo1.servoClosed();
      delay(1000);
      /* Debug Code Start*/
      Serial.print("Move Servo 1 to open: ");
      Serial.println(testServo1.z_servo_micro_current);   
      /* Debug Code End */
      testServo1.servoOpen();
      delay(1000);
      /* Debug Code Start*/
      Serial.print("Move Servo 2 to closed: ");
      Serial.println(testServo2.z_servo_micro_current);  
      /* Debug Code End */
      testServo2.servoClosed();
      delay(1000);
      /* Debug Code Start*/
      Serial.print("Move Servo 2 to open: ");
      Serial.println(testServo2.z_servo_micro_current);   
      /* Debug Code End */
      testServo2.servoOpen();
      delay(1000);
      break;
    default:
      Serial.println("Unintended State. Please Check Source Code.");
      delay(1000);
  }
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
void clickRightISR()
{
  clickRightTrigger = HIGH;
}
void clickLeftISR()
{
  clickLeftTrigger = HIGH; 
}
