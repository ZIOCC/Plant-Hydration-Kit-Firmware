/*
 * Plant Hydrating Kit firmware
 * Before uploading this code, you need to install the Adafruit PWMServoDriver library
 * 
 * This code is created by Aswana for Smart Prototyping
 * For more info contact aswana@smart-prototyping.com
 * 
 * Check out our blogpost on how to set up at 
 * https://www.smart-prototyping.com/blog/tinkertuesday-plant-hydrating-kit
 * */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN  1000 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  3000 // this is the 'maximum' pulse length count (out of 4096)
#define MOTOR 0x40

//define for soil moisture sensor
#define COMMAND_LED_OFF     0x00
#define COMMAND_LED_ON      0x01
#define COMMAND_GET_VALUE        0x05
#define COMMAND_NOTHING_NEW   0x99

Adafruit_SSD1306 display(OLED_RESET);
Adafruit_PWMServoDriver pwm;
const byte qwiicAddress = 0x28;     //Default Address for soil moisture sensor
uint16_t ADC_VALUE= 0;


/******YOU CAN MAKE CHANGES HERE**************************/

//Set your moisture level here
uint16_t DRY= 650;
uint16_t WET= 250;
uint16_t NORMAL= 500;

//Set the time needed to check plant moisture to water your plant again
uint16_t DELAY= 3600000;//1 hour in milliseconds interval

//set the time needed for your sensor to checks again and reconfirm the value
uint16_t CONFIRM= 60000;//5 minutes in milliseconds interval

// our servo # counter
uint16_t pulselen = 4000;

/***********END TO MAKING CHANGES**********************/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  display.clearDisplay();
  delay(1000);
  Serial.println("Zio Qwiic Soil Moisture Sensor Master Awake");
  //setup for soil moisture sensor
  Wire.begin();
  testForConnectivity();
  ledOn();
  delay(500);
  
  //setup for the motor driver
  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  yield();

}

void  goahead()
{
  pwm.setPin(3,0,1);    //H  STBY1  
  pwm.setPin(1,0,1);    //H
  pwm.setPin(2,4095,1); //L
  pwm.setPin(4,0,1);    //H        
  pwm.setPin(5,4095,1); //L
    
  pwm.setPin(10,0,1);    //H  STBY2  
  pwm.setPin(8,0,1);    //H
  pwm.setPin(9,4095,1); //L
  pwm.setPin(11,0,1);    //H        
  pwm.setPin(12,4095,1); //L
  }

void  goback()
{
  pwm.setPin(3,0,1);    //H  STBY1  
  pwm.setPin(2,0,1);    //H
  pwm.setPin(1,4095,1); //L
  pwm.setPin(5,0,1);    //H        
  pwm.setPin(4,4095,1); //L
    
  pwm.setPin(10,0,1);    //H  STBY2  
  pwm.setPin(9,0,1);    //H
  pwm.setPin(8,4095,1); //L
  pwm.setPin(12,0,1);    //H        
  pwm.setPin(11,4095,1); //L
  }  

 void stp()
 {
  pwm.setPin(3,0,0);     //L  STBY1
  pwm.setPin(10,0,0);    //L  STBY2
  }


void loop() {
  // put your main code here, to run repeatedly:
//  display.clearDisplay();

//  delay(1000);
  waterPlant();
}


// LED is off, and a -1 if an error occurred.
void get_value() {
  display.clearDisplay();
  Wire.beginTransmission(qwiicAddress);
  Wire.write(COMMAND_GET_VALUE); // command for status
  Wire.endTransmission();    // stop transmitting //this looks like it was essential.

  Wire.requestFrom(qwiicAddress, 2);    // request 1 bytes from slave device qwiicAddress

  while (Wire.available()) { // slave may send less than requested
  uint8_t ADC_VALUE_L = Wire.read(); 
  uint8_t ADC_VALUE_H = Wire.read();
  ADC_VALUE=ADC_VALUE_H;
  ADC_VALUE<<=8;
  ADC_VALUE|=ADC_VALUE_L;
  Serial.print("Soil Moisture:  ");
  Serial.println(ADC_VALUE,DEC);
  display.setTextSize(1);//set the size of the text
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Soil Moisture:  ");
      display.println(ADC_VALUE,DEC);
      display.display();
   }
  uint16_t x=Wire.read(); 
}

void ledOn() {
  Wire.beginTransmission(qwiicAddress);
  Wire.write(COMMAND_LED_ON);
  Wire.endTransmission();
}

void ledOff() {
  Wire.beginTransmission(qwiicAddress);
  Wire.write(COMMAND_LED_OFF);
  Wire.endTransmission();
}


// testForConnectivity() checks for an ACK from an Sensor. If no ACK
// program freezes and notifies user.
void testForConnectivity() {
  Wire.beginTransmission(qwiicAddress);
  //check here for an ACK from the slave, if no ACK don't allow change?
  if (Wire.endTransmission() != 0) {
    Serial.println("Check connections. No slave attached.");
    while (1);
  }
}

void warn(){
       ledOn();
       delay(100);
       ledOff();
       delay(100);
}

void waterStart(){
  goahead();
  pwm.setPWM(0, 0, pulselen);
  Serial.println("watering start");
}

void confirmValue(){
     delay(CONFIRM);//15 minutes delay
     get_value(); //checks plant moisture again to confirm
      
      if (ADC_VALUE > DRY){
      Serial.println("Your plant is thirsty!");
      warn();
      }
      else if (ADC_VALUE > NORMAL && ADC_VALUE < DRY){
      ledOn();
      Serial.println("Your plant is almost thirsty");   
      }
      else {
      Serial.println("Your plant is hydrated!");
      }
}


void waterPlant(){

   get_value();//Checks your plant's moisture level
   
   if (ADC_VALUE >= DRY) {
     
    confirmValue();
    waterStart();
    
   }    
   else if (ADC_VALUE < DRY) {

    confirmValue();
    stp();
        
    }
   else {
    
    stp();
    delay(DELAY);
    
   }
 }

