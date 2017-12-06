#include "Adafruit-WS2801-Library/Adafruit_WS2801.h"
#include <Wire.h>                         // The Display Shield needs Wire for its I2C communication.  It comes with Arduino
#include <Adafruit_RGBLCDShield.h>        // Then Adafruit provides this library to talk to the shield.  Find this at Adafruit
#include <physMenu.h>                     // This is is our library used by this menu template.  Find this at GC

PhysicsMenu m = PhysicsMenu();

// MIDI variables
int whiteKeys[] = {0x24,0x26,0x28,0x29,0x2B,0x2D,0x2F,0x30,0x32,0x34,0x35,0x37,0x39,0x3B,0x3C,0x3E,0x40,0x41,0x43,0x45,0x47,0x48,0x4A,0x4C,0x4D,0x4F,0x51,0x53,0x54};
int pentatonic[] = {0x24, 0x26, 0x28, 0x2B, 0x2D, 0x30, 0x32, 0x34, 0x37, 0x39, 0x3C, 0x3E, 0x40, 0x43, 0x45, 0x48, 0x4A, 0x4C, 0x4F, 0x51, 0x54, 0x56, 0x58, 0x5B, 0x5D, 0x60};
int tempo = 60;
//=====================================================

int dataPin = 2;
int clockPin = 3;
Adafruit_WS2801 strip = Adafruit_WS2801(25, dataPin, clockPin);
uint32_t pixelOff;
uint32_t tempColor;
int range;
int floatingFollowerSharpness = 2;
int howCloseToRange;
byte testByte = B11111111;
byte testByte2 = B00000000;
uint32_t colorBitMask;
uint32_t resultB32;

// rangefinders:
pinMode(6, OUTPUT);
pinMode(7, INPUT);
pinMode(9, OUTPUT);
pinMode(10, INPUT);

//=====================================================

void setup() {
  // put your setup code here, to run once:
  Serial.begin(31250);

  m.begin();
  m.addItem("1 Set Tempo  ", &setTempo);
  m.addItem("2 Pentatonic ", &playPent);
  m.addItem("3 C Major    ", &playCMajor);
  m.addItem("4 LED Test   ", &ledTest);
  m.addItem("5 RangeTest  ", &rangeTest);
  m.addItem("6 RangeTest2 ", &rangeTest2);
  m.addItem("7 NewFollower", &newFollower);
  m.lcd.setBacklight(0x1);

  strip.begin(); // starts the LED strip
  pixelOff = Color(0,0,0);
  strip.show();
  colorBitMask = Color(252,252,252);
}

void loop() {
  // put your main code here, to run repeatedly:
  m.dispatch();
}

// MENU FUNCTIONS ==========================================================

void noteOn(int pitch) {
  Serial.write(0x90);
  Serial.write(pitch);
  Serial.write(0x45);
}

void noteOff() {
  Serial.write(80);
}

void setTempo(){
  m.lcd.clear();
  while(1) {
    m.lcd.home();
    m.lcd.print("Tempo = ");
    m.lcd.print(tempo);  
    m.lcd.print(" bpm ");
    delay(100); // This slows counting when a button is held down
    byte buttons = m.getButtons(); // 'buttons' value depends on what was presssed
    if (buttons == BUTTON_UP) {tempo += 30;}  // Up button increments menu option
    if (buttons == BUTTON_DOWN) {tempo -= 30;} // Down button decrements menu option
    if (tempo < 30) {tempo = 30;}
    if (tempo > 240) {tempo = 240;}
    if (buttons == BUTTON_LEFT) {return;}
  }
}


void arpegChord(int root){
  Serial.write(0x90);
  Serial.write(whiteKeys[root]);
  Serial.write(0x40);
  delay(250);
  Serial.write(0x90);
  Serial.write(whiteKeys[root + 2]);
  Serial.write(0x40);
  delay(250);
  Serial.write(0x90);
  Serial.write(whiteKeys[root + 4]);
  Serial.write(0x40);
}
void fullChord(int root){
  Serial.write(0x90);
  Serial.write(whiteKeys[root]);
  Serial.write(0x40);
  Serial.write(0x90);
  Serial.write(whiteKeys[root + 2]);
  Serial.write(0x40);
  Serial.write(0x90);
  Serial.write(whiteKeys[root + 4]);
  Serial.write(0x40);
}

void playCMajor(){
  m.lcd.clear();
  bool arpegStatus = 1;
  bool cycle = 0;
  while(1){
    byte buttons = m.getButtons();
    if (buttons == BUTTON_SELECT){
      if (arpegStatus == 0){
        arpegStatus = 1;
      }
      else{
        arpegStatus = 0;
      }
    }
    int root = map(constrain(getRange(), 0, 100), 0, 100, 0, 29);
    int note = map(constrain(getRange2(), 0, 100), 0, 100, 0, 29);
    if (root < 29){
      if (cycle == 0){
        cycle = 1;
        noteOn(whiteKeys[note]);
        if (arpegStatus == 0){
          fullChord(root);
        }
        else{
          arpegChord(root);
        }
      }
      else{
        noteOn(whiteKeys[note]);
        cycle = 0;
      }
    }
    int tempoDelay = map(tempo, 60, 240, 1000, 250);
    delay(tempoDelay/2);
    if (buttons == BUTTON_LEFT) {
      noteOff();
      return; 
    }
  }
}

void playPent() {
  bool cycle = 0;
  m.lcd.clear();
  while(1){
    int noteRange = getRange();
    int chordRange = getRange2();
    int adjustedNRange = constrain(noteRange, 0, 100);
    int adjustedCRange = constrain(chordRange, 0, 100);
    int note = map(adjustedNRange, 0, 100, 0, 26);
    int chord = map(adjustedCRange, 0, 100, 0, 26);
    if (note < 26 && chord < 26){
      if (cycle == 0){
        cycle = 1;
        noteOn(pentatonic[note]);
        noteOn(pentatonic[chord + 2]);
        noteOn(pentatonic[chord]);
        noteOn(pentatonic[chord - 2]);
      }
      else{
        noteOn(pentatonic[note]);
        cycle = 0;
      }
    int tempoDelay = map(tempo, 60, 240, 1000, 250);
    delay(tempoDelay/2);
    }   
    byte buttons = m.getButtons();
    if (buttons == BUTTON_LEFT) {
      noteOff();
      return;     
    } 
  }
}


void ledTest() {
  m.lcd.clear();
  while(1) {
    rainbow(20);
  }
}

void rangeTest() {
  m.lcd.clear();
  while(1) {
    range = getRange();
    m.lcd.home();
    //m.lcd.print(range);
    //m.lcd.print("     ");
    colorFollower(Color(155,155,155), range);
    byte buttons = m.getButtons();
    if (buttons == BUTTON_LEFT) {
      return;
    }
  }
}

// max: 75cm
// min: 0cm


void rangeTest2() {
  m.lcd.clear();
  while(1) {
    range = getRange();
    m.lcd.home();
    //m.lcd.print(range);
    //m.lcd.print("     ");
    floatingFollower(Color(255,255,255), range);
    //rangeNote(range);
    byte buttons = m.getButtons();
    if (buttons == BUTTON_LEFT) {
      return;
    }
  }
}

void newFollower(uint32_t c) {
  m.lcd.clear();
  m.lcd.home();
  int newRange = 0;
  int oldRange;
  for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, pixelOff);
  }
  strip.show();
  uint32_t halfBrightness = colorDivider(c, 1);
  uint32_t quarterBrightness = colorDivider(c, 2);
  uint32_t eighthBrightness = colorDivider(c, 3);
  uint32_t sixteenthBrightness = colorDivider(c, 4);
  
  while(1) {
    oldRange = newRange;
    newRange = getRange()/4;
    strip.setPixelColor(oldRange-4, pixelOff);
    strip.setPixelColor(oldRange-3, pixelOff);
    strip.setPixelColor(oldRange-2, pixelOff);
    strip.setPixelColor(oldRange-1, pixelOff);
    strip.setPixelColor(oldRange, pixelOff);
    strip.setPixelColor(oldRange+1, pixelOff);
    strip.setPixelColor(oldRange+2, pixelOff);
    strip.setPixelColor(oldRange+3, pixelOff);
    strip.setPixelColor(oldRange+4, pixelOff);
    strip.setPixelColor(newRange-4, sixteenthBrightness);
    strip.setPixelColor(newRange-3, eighthBrightness);
    strip.setPixelColor(newRange-2, quarterBrightness);
    strip.setPixelColor(newRange-1, halfBrightness);
    strip.setPixelColor(newRange, c);
    strip.setPixelColor(newRange+1, halfBrightness);
    strip.setPixelColor(newRange+2, quarterBrightness);
    strip.setPixelColor(newRange+3, eighthBrightness);
    strip.setPixelColor(newRange+4, sixteenthBrightness);
    strip.show();
    delay(40);
  }
}

void proxGlow(uint32_t color) {
  m.lcd.clear();
  m.lcd.home();
  int glowRange;
  while(1) {
    glowRange = getRange()/4;
    if(glowRange <= 25) {
      moddedColor = color;
    } else if(glowRange <= 50) {
      moddedColor = colorDivider(color,2);
    } else if(glowRange <= 75) {
      moddedColor = colorDivider(color,3);
    } else if(glowRange <= 100) {
      moddedColor = colorDivider(color,4);
    } else {
      moddedColor = colorDivider(color,5);
    }
    
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, moddedColor);
    }
    strip.show();
  }
}


void triRangeCircle(uint32_t color) {
  m.lcd.clear();
  m.lcd.home();
  int range1;
  int range2;
  int range3;
  int mostDist;
  int averageRange;
  int LEDposition;
  
  while(1) {
    range1 = getRange();
    range2 = getRange2();
    range3 = getRange3();

    if(range1 < range2) {
      if(range2 < range3) {
        mostDist = 3;
      } else {mostDist = 2}
    } else if(range1 > range3) {
      mostDist = 1;
      }             // determines which rangefinder has the least activity / which rangefinder is farthest from an object
    
    switch(mostDist) {
      case 1:
        range1 = 0;
      case 2:
        range2 = 0;
      case 3:
        range3 = 0;
    }           // could eliminate this to save time by moving rangeX = 0 directly into the previous if statements.

    averageRange = (range1 + range2 + range3)/2;
    moddedColor = colorDivider(color, averageRange/100); // set brightness based on how close the object is to two closest rangefinders

    // REST OF FUNCTION GOES HERE (find position of object in ring)
  }
}

// LED HELPER FUNCTIONS ====================================================

void colorWipe(uint32_t c, uint8_t wait) {
  int i;
 
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void dualColorWipe(uint32_t ca, uint32_t cb, uint8_t wait) {
  int i;
  bool toggle = false;
 
  for (i=0; i < strip.numPixels(); i++) {
      if (toggle == false) {
        strip.setPixelColor(i, ca);
        strip.show();
        toggle = true;
      }
      else if (toggle == true) {
        strip.setPixelColor(i, cb);
        strip.show();
        toggle = false;
      }
      delay(wait);
      noteOff();
  } 
}

void dualColorToneWipe(uint32_t ca, uint32_t cb, uint8_t wait) {
  int i;
  bool toggle = false;
 
  for (i=0; i < strip.numPixels(); i++) {
      if (toggle == false) {
        strip.setPixelColor(i, ca);
        strip.show();
        toggle = true;
      }
      else if (toggle == true) {
        strip.setPixelColor(i, cb);
        strip.show();
        toggle = false;
      }
      delay(wait);
      noteOff();
  }
}

void colorFollower(uint32_t c, int givenRange) {
  int i;
  int adjustedRange = givenRange/3;
  
  for (i=0; i < adjustedRange; i++) {
      strip.setPixelColor(i, c);
    }
  for (i=adjustedRange; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, pixelOff);
    }  
  strip.show();
}


void floatingFollower(uint32_t c, int givenRange) {
  int i;
  floatingFollowerSharpness = 2;
  int adjustedRange = givenRange/3;
  
  for (i=0; i < strip.numPixels(); i++) {
      howCloseToRange = abs(adjustedRange-i);
      if (howCloseToRange <= floatingFollowerSharpness) {
        tempColor = colorDivider(c,howCloseToRange);
        strip.setPixelColor(i, tempColor);
      }
      else {
        strip.setPixelColor(i, pixelOff);
      }
    }
  strip.show();
}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

uint32_t colorDivider(uint32_t inputColor, int powerOf2)
{
  int dimmerValue;
  dimmerValue = simplePower(powerOf2)-1;
  int brightVal = 255 - dimmerValue;
  uint32_t bitMask = Color(brightVal,brightVal,brightVal);
  uint32_t result = inputColor & bitMask;
  result = result >> powerOf2;
  return result;
}


void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 256; j++) {     // 3 cycles of all 256 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 255));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


//=========================== Miscellaneous Functions ==========================

int getRange() {
  //pinMode(6, OUTPUT);
  //pinMode(7, INPUT);
  digitalWrite(6, HIGH);
  delayMicroseconds(10);
  digitalWrite(6, LOW);
  int message_time = pulseIn(7, HIGH);
  int range = message_time/58.0;
  return range;
}

int getRange2() {
  //pinMode(9, OUTPUT);
  //pinMode(10, INPUT);
  digitalWrite(9, HIGH);
  delayMicroseconds(10);
  digitalWrite(9, LOW);
  int message_time = pulseIn(10, HIGH);
  int range = message_time/58.0;
  return range;
}

int getRange3() {
  //pinMode(13, OUTPUT);
  //pinMode(14, INPUT);
  digitalWrite(13, HIGH);
  delayMicroseconds(10);
  digitalWrite(13, LOW);
  int message_time = pulseIn(14, HIGH);
  int range = message_time/58.0;
  return range;
}

int simplePower(int v) {
  switch(v) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 4;
    case 3:
      return 8;
    case 4:
      return 16;
    case 5:
      return 32;
    case 6:
      return 64;
    case 7:
      return 128;
  }
}
