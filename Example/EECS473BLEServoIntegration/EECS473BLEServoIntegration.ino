#include <EECS473Servo.h>
#include <EECS473BLECombo.h>
#include <EMGLDAWrapper.h>

/*
----------------------------------
-----ISR Test Function Prototypes-----
----------------------------------
*/
void modeISR();
void clickRightISR();
void clickLeftISR();

// classification function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void awaitMyoSequence();

/*
----------------------------------
-------------Globals------------
----------------------------------
*/
// Globals for LED and button pins
const int buttonModePin = 15;
const int ledGreenPin =  27;  
const int ledBluePin = 26;
// Globals that will control button interrupts and corresponding for-loop functionality
volatile byte modeState = 0;
volatile byte modeTrigger = LOW;
// globals for classification
uint8_t output;

// Class instantiation
BLEClass TestBLE;
ArmServo testServo1;  // ti
ArmServo testServo2;  // mrp
myoLDAComboClass Test;

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
  byte status = TestBLE.init();
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
      awaitMyoSequence();
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledBluePin, LOW);
      if(TestBLE.comboKeyboard.isConnected())
      {
        // Serial.println("Sending 'Hello world'");
        // TestBLE.comboKeyboard.println("Hello World");
        // Serial.println("Sending Enter key...");
        // TestBLE.comboKeyboard.write(KEY_RETURN);
        Serial.println("Sending gesture-recognized character: " + (char)output);
        TestBLE.comboKeyboard.write((char)output);
        delay(100);
      }
      else
      {
        Serial.println("BLE Keyboard/Mouse not connected. Waiting 2 seconds...");
        delay(2000);
      }
    break;
    case 1:
      awaitMyoSequence();
      digitalWrite(ledGreenPin, LOW);
      digitalWrite(ledBluePin, HIGH);
      if(TestBLE.comboKeyboard.isConnected())
      {
        TestBLE.mouseMove();
        if(output == 0x65)
        {
          TestBLE.comboMouse.click(MOUSE_RIGHT);
        }
        else if(output == 0x20)
        {
          TestBLE.comboMouse.click(MOUSE_LEFT);
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
      int gesture = makeMyoPredictions();
      switch(gesture){
        case 1:
          Serial.println("Making fist gesture");
          testServo1.close();
          testServo2.close();
          break;
        case 2:
          Serial.println("Making fingergun gesture");
          testServo1.open();
          testServo2.close();
          break;
        case 3:
          Serial.println("Making openhand gesture");
          testServo1.open();
          testServo2.open();
          break;
        case 4:
          Serial.println("Making okhand gesture");
          testServo1.close();
          testServo2.open();
          break;
        default:
          Serial.println("Invalid gesture for servo control");
          break;
      }
      delay(100);
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

/*
  Gather's data from the serial bus and parses it into an output code.
*/

void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) 
{
  Test.emgstreamer.streamData(pData, length);
}

void awaitMyoSequence(){
  if (!Test.myo.connected) 
    Test.setupMyo();
  else
  {
      /*Gather Predictions*/
      Test.bluetoothGestureSequence(Test.buff);
      /*Output Predictions*/
      Serial.print("Outputted Buffer: ");
      Serial.print(Test.buff[0]);
      Serial.print(Test.buff[1]);
      Serial.println(Test.buff[2]);
      /*Format and output predictions into new value*/
      output = Test.parse_gestures(Test.buff);
      Serial.print("Outputted Bluetooth Keypress: ");
      Serial.println((char)output);
  }
}