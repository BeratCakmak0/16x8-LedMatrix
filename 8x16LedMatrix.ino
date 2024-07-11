#include <avr/pgmspace.h>
#include "font8x8.h"
#include "image.h"
const int latchPin = 0;//define latch pin
const int dataPin = 1;//define dataPin pin
const int clockPin = 2;//define clockPin pin
const int blankPin = 3;//define blankPin pin

const uint8_t rows[8] = {B00000001,B00000010,B00000100,B00001000,B00010000,B00100000,B01000000,B10000000};//define row reading order
unsigned long previousMillis;
const long shiftDelay = 50;//define how fast text shift
const int whichMode = 1;//mode 1 = show image mode 2 = scrolling text 
String rowText = "Lorem Ipsum";//define what we want to write 

void setup() {
  pinMode(latchPin,OUTPUT);
  pinMode(dataPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(blankPin,OUTPUT);
  digitalWrite(blankPin, LOW);
}

void loop() {
  if(whichMode == 1){
    ShowImage();
  }else{
    ScrollingText();
  }
}
void ShowImage(){
  static uint8_t regData1,regData2;//define variable for register datas
  
  for(int i = 0;i < 8;i++){
    regData1 = pgm_read_byte(&image[0][i]);//get datas from font file
    regData2 = pgm_read_byte(&image[1][i]);//get datas from font file

    digitalWrite(latchPin,LOW);//set latch pin to 0 for data sending
    shiftOut(dataPin,clockPin,MSBFIRST,~regData1);//send data
    shiftOut(dataPin,clockPin,MSBFIRST,~regData2);//send data
    shiftOut(dataPin,clockPin,MSBFIRST,~rows[i]);//send data
    digitalWrite(latchPin,HIGH);//set latch pin to 1 for data sending is done
  }
}

void ScrollingText(){
  static int rightShift = 16;
  static int leftShift,letterCounter = 0;
  static uint8_t regData1,regData2,regData3;//define variable for register datas
  unsigned long currentMillis = millis();

  for(int i = 0;i < 8;i++){
    regData1 = pgm_read_byte(&font8x8[rowText.charAt(letterCounter)-32][i]);//get datas from font file
    regData2 = pgm_read_byte(&font8x8[rowText.charAt(letterCounter+1)-32][i]);//get datas from font file
    regData3 = pgm_read_byte(&font8x8[rowText.charAt(letterCounter+2)-32][i]);//get datas from font file

    currentMillis = millis();
    if(currentMillis - previousMillis >= shiftDelay){
      previousMillis = currentMillis;
      if(rightShift != 0){
        rightShift --;
      }else{
        leftShift ++;
        if(leftShift == 8){
          letterCounter ++;
          leftShift = 0;
        }
      }
    }
    
    if(letterCounter >= rowText.length()-2){
       regData3 = 0b00000000;
    }
    if(letterCounter >= rowText.length()-1){
       regData3 = 0b00000000;
       regData2 = 0b00000000;
    }
    if(letterCounter >= rowText.length()){
       regData3 = 0b00000000;
       regData2 = 0b00000000;
       regData1 = 0b00000000;
       letterCounter = 0;
       leftShift = 0;
       rightShift=16;
    }
    
    for(int k = 0;k<rightShift;k++){//right shift
      regData3 = regData3 >> 1;
      bitWrite(regData3,7,bitRead(regData2,0));
      regData2 = regData2 >> 1;
      bitWrite(regData2,7,bitRead(regData1,0));
      regData1 = regData1 >> 1;
    }
    
    if(rightShift <= 0){
      for(int k = 0;k<leftShift;k++){//left shift
        regData1 = regData1 << 1;
        bitWrite(regData1,0,bitRead(regData2,7));
        regData2 = regData2 << 1;
        bitWrite(regData2,0,bitRead(regData3,7));
        regData3 = regData3 << 1;
      }
    }

    digitalWrite(latchPin,LOW);
    shiftOut(dataPin,clockPin,MSBFIRST,~regData1);
    shiftOut(dataPin,clockPin,MSBFIRST,~regData2);
    shiftOut(dataPin,clockPin,MSBFIRST,~rows[i]);
    digitalWrite(latchPin,HIGH);
  }
}
