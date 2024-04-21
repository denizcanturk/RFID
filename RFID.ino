/**
 * Author : Deniz CANTURK
 * RFID Card Reader and Door Opener etc.
*/

#include <SoftwareSerial.h>         
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9

Servo doorLock;
uint8_t servoPin = 3;
                       
uint8_t greenLightPin = 4;
uint8_t redLightPin = 5;
uint8_t presenceSensorRead = 8;
uint8_t i=0;
byte presenceSensorValue=0;
byte prevPresenceSensorValue=0;
uint8_t servoPos = 50;

bool isAuthorized = false;
bool toRight = true;
bool animeStart = true;
bool cardRead = false;
bool blinkLEDs = true;
bool doorState = false;

byte readCard[4];
String tag_UID = "1392ABD";  // My UID for the TAG - We can create a list of multiple TAGs later on...
String tagID = "";

MFRC522 mfrc522(SS_PIN, RST_PIN); 
LiquidCrystal_I2C lcd(0x27,16,2);

unsigned long prevTime = 0;
unsigned long currentTime = 0;
unsigned long readStateTime = 0;
unsigned long animeStopTime = 0;
unsigned long ledTime = 0;

uint16_t INTERVAL = 2000;
uint16_t LED_INTERVAL = 750;

void setup() {
  Serial.begin(9600);               
  pinMode(presenceSensorRead, INPUT);
  pinMode(greenLightPin, OUTPUT);
  pinMode(redLightPin, OUTPUT);

  SPI.begin(); 
  mfrc522.PCD_Init();
  
  lcd.init();                 
  lcd.backlight();                
  lcd.home();
  lcd.clear();
  doorLock.attach(3);
  doorLock.write(servoPos);
}
 
void loop() {
  currentTime = millis();
  readSensor();
  scanAnimation();
}


void readSensor(){
  presenceSensorValue = digitalRead(presenceSensorRead)^1;
  welcome();
  /*Object detection in close range...*/
//  if (presenceSensorValue == 1){
//    
//    animeStopTime = currentTime;
//    animeStart = false;
//    prevPresenceSensorValue = presenceSensorValue;
//  }
//  else {
//    animeStart = true;
//  }
    
  ledBlinker(redLightPin);
  
    while (readID()){
      blinkLEDs = false;
      cardRead = true;
      readStateTime = currentTime;
      if (tagID == tag_UID){
        isAuthorized=true;
      }
      else{
        isAuthorized=false;
      }
    }
    /*
     *For the name like "Gazanfar, 
     * we can define multiple tag and 
     * associated names to be displayed
     * in the screen 
     */
    if (isAuthorized == true && cardRead ==true){
      if(currentTime - readStateTime < INTERVAL){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Gazanfar icin:");
        lcd.setCursor(0,1);
        lcd.print("Giris Onaylandi!");
        digitalWrite(redLightPin, LOW);
        digitalWrite(greenLightPin, HIGH);
        opendoor();
        delay(4000);
        
        resetVals();
      }
    }
    else{
        if (currentTime - readStateTime < INTERVAL && cardRead ==true){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("XxOnaylanmadi!xX");
          digitalWrite(greenLightPin, LOW);
          digitalWrite(redLightPin, HIGH);
          delay(3000);
          resetVals();
        }
      }
}
/* Reseting everything to initial states. */
//---------------------------------
void resetVals(){
  isAuthorized = false;
  blinkLEDs = true;
  readStateTime = currentTime;
  prevPresenceSensorValue=0;
  cardRead = false;
  tagID = "";
  animeStart = true;
  digitalWrite(greenLightPin, LOW);
  digitalWrite(redLightPin, LOW);
  closeDoor();
}

/* Presence Detector, detection case */
//---------------------------------

void welcome()
{
    if (currentTime - animeStopTime > 1000){
      if (animeStart)
      {
        lcd.setCursor(0,0);
        lcd.print("  NOVA Academy  ");
        lcd.setCursor(0,1);
        lcd.print("  HOSGELDINIZ!  ");
      }
      animeStart = true;
    }
}

/* Just a small effect on general */
//---------------------------------

void ledBlinker(int pin)
{
    if (currentTime - ledTime > 1000){
      if (blinkLEDs)
      {
        digitalWrite(pin, digitalRead(pin)^1);        
      }
        ledTime = currentTime;
      }
}

/* Just an effect on Screen 
 * Can be removed if not interested */
//---------------------------------

void scanAnimation(){
  if (animeStart)
  {
    if (toRight)
    {
      lcd.clear();
      lcd.setCursor(i,0);
      lcd.write(0);
      lcd.setCursor(i,1);
      lcd.write(0);
      delay(100);
      
      lcd.clear();
      lcd.setCursor(i,0);
      lcd.write(11);
      lcd.setCursor(i,1);
      lcd.write(11);
      delay(100);  
      i++;
      if(i==16)
        toRight = false;
    }
    else
    {
      i--;
      lcd.clear();
      lcd.setCursor(i,0);
      lcd.write(0);
      lcd.setCursor(i,1);
      lcd.write(0);
      delay(100); 
      
      lcd.clear();
      lcd.setCursor(i,0);
      lcd.write(11);
      lcd.setCursor(i,1);
      lcd.write(11);
      delay(100);  
      if(i==0)
        toRight = true;
    }
  }
}

//---------------------------------
/* Read the TAG and assigns the value to global variable for access ease. */
boolean readID()
  {
    //Check if a new tag is detected or not. If not return.
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      return false;
    }
    //Check if a new tag is readable or not. If not return.
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      return false;
    }
    tagID = "";
    // Read the 4 byte UID
    for ( uint8_t i = 0; i < 4; i++)
    {
      //readCard[i] = mfrc522.uid.uidByte[i];
      tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Convert the UID to a single String
    }
    tagID.toUpperCase();
    mfrc522.PICC_HaltA(); // Stop reading
    return true;
  }

void opendoor(){
  if (doorState == false){
  for (int i = 0; i < 55; i+=1){
    servoPos +=1;
    doorLock.write(servoPos);
    delay(30);
  }
  doorState = true;
  }
}

void closeDoor(){
  if (doorState == true){
  for (int i = 0; i < 55; i+=1){
    servoPos -= 1;
    doorLock.write(servoPos);
    delay(30);
  }
  doorState = false;
  }
}