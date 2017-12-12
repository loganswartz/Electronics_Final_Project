/* 
 *  INTERACTIVE LIGHT & SOUND
 *  By Spencer Aeschliman and Logan Swartzendruber
 *  
 *  This project is hosted as a GitHub repo at https://github.com/loganswartz/Electronics_Final_Project, including
 *  all the necessary libraries.
 */

#include <Adafruit_WS2801.h>              // Included in GitHub repo or find it at https://learn.adafruit.com/12mm-led-pixels/code
#include <Wire.h>                         // The Display Shield needs Wire for its I2C communication.  It comes with the Arduino IDE.
#include <Adafruit_RGBLCDShield.h>        // Then Adafruit provides this library to talk to the shield. Find this included in the GitHub repo or at https://learn.adafruit.com/rgb-lcd-shield/using-the-rgb-lcd-shield
#include <physMenu.h>                     // This is is our library used by this menu template.  Find this included in GitHub repo or at GC

PhysicsMenu m = PhysicsMenu();     // create menu object

// MIDI variables
int whiteKeys[] = {0x24,0x26,0x28,0x29,0x2B,0x2D,0x2F,0x30,0x32,0x34,0x35,0x37,0x39,0x3B,0x3C,0x3E,0x40,0x41,0x43,0x45,0x47,0x48,0x4A,0x4C,0x4D,0x4F,0x51,0x53,0x54};
int pentatonic[] = {0x24, 0x26, 0x28, 0x2B, 0x2D, 0x30, 0x32, 0x34, 0x37, 0x39, 0x3C, 0x3E, 0x40, 0x43, 0x45, 0x48, 0x4A, 0x4C, 0x4F, 0x51, 0x54, 0x56, 0x58, 0x5B, 0x5D, 0x60};
int tempo = 60;

// Pins used for controlling the RGB LED light strand:
int dataPin = 2;
int clockPin = 3;
Adafruit_WS2801 strip = Adafruit_WS2801(25, dataPin, clockPin);    // creates a "strip" object from the WS2801 Adafruit library

// variables for various LED functions
int range;
int howCloseToRange;
int floatingFollowerSharpness;

byte testByte = B11111111;
byte testByte2 = B00000000;

uint32_t pixelOff;
uint32_t tempColor;
uint32_t colorBitMask;
uint32_t resultB32;
uint32_t moddedColor;


//=====================================================

void setup() {

  // we initialize the LCD shield menu and the Serial connection to use with the MIDI box
  Serial.begin(31250);  // the MIDI box uses this specific baud rate, anything else and the sound won't work

  m.begin();
  m.addItem("1 Set Tempo     ", &setTempo);
  m.addItem("2 Pentatonic    ", &playPent);
  m.addItem("3 C Major       ", &playCMajor);
  m.addItem("4 LED Test      ", &ledTest);
  m.addItem("5 RangeTest     ", &rangeTest);
  m.addItem("6 NewFollower   ", &newFollower);
  m.addItem("7 Color Changer ", &rangeColor);
  m.addItem("8 ProxGlow      ", &proxGlower);
  m.addItem("9 TriRangeGlow  ", &triRangeGlow);
  m.lcd.setBacklight(0x1);

  strip.begin();  // starts the LED strip
  pixelOff = Color(0,0,0);  // predefine a color variable that is "off"
  strip.show();
  colorBitMask = Color(252,252,252);  // create a mask that is B11111100 times 3 (111111001111110011111100)


  // sets mode of pins to be used by rangefinders:
  pinMode(6, OUTPUT);
  pinMode(7, INPUT);
  pinMode(9, OUTPUT);
  pinMode(10, INPUT);
  pinMode(11, OUTPUT);
  pinMode(12, INPUT);
}

void loop() {
  m.dispatch();
}



// MENU FUNCTIONS ==========================================================


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

void playCMajor(){
  m.lcd.clear();
  bool chordStatus = 0;
  bool cycle = 0;
  while(1){
    byte buttons = m.getButtons();
    if (buttons == BUTTON_SELECT){
      if (chordStatus == 0){
        chordStatus = 1;
      }
      else{
        chordStatus = 0;
      }
    }
    int root = map(constrain(getRange3(), 0, 100), 0, 100, 0, 29);
    int note = map(constrain(getRange2(), 0, 100), 0, 100, 0, 29);
    if (root < 29){
      if (chordStatus == 0){
        noteOn(whiteKeys[note]);
      }
      else{
        if (cycle == 0){
          noteOn(whiteKeys[note]);
          cycle = 1;
        }
        else{
          noteOn(whiteKeys[note]);
          fullChord(root);
          cycle = 0;
        }
      }
    }
    int tempoDelay = map(tempo, 60, 240, 1000, 250);
    delay(tempoDelay/2);
    //delay(250);
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

/*
 * pretty colors
 */
void ledTest() {
  m.lcd.clear();
  while(1) {
    rainbow(20);
  }
}

/*
 * One rangefinder controls the number of lights that are lit up
 * and the note that is played (colorFollower and noteOn) and the
 * other controls the color of the lights (createColor; the value
 * of range2 is determined inside createColor, not in rangeTest.)
 */
void rangeTest() {
  m.lcd.clear();
  while(1) {
    range = getRange3();
    m.lcd.home();
    //m.lcd.print(range);
    //m.lcd.print("     ");
    colorFollower(createColor(), range);
    noteOn(whiteKeys[createRoot()]);
    delay(50);
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

/*
 * Improved iteration on "orb" style follower, much faster than floatingFollower.
 * Implementation in triRangeCircle of turning the lights on and off is even
 * less resource intensive than this, but only ever so slightly, because it
 * uses only one range variable instead of 2.
 */
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

void proxGlower() {
  m.lcd.clear();
  m.lcd.home();
  proxGlow(Color(255,255,255));
}

void triRangeGlow() {
  m.lcd.clear();
  m.lcd.home();
  triRangeCircle(Color(255,255,255));
}



// LED HELPER FUNCTIONS ====================================================


// Create a packed 24-bit color value from R,G,B
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

/*
 * colorDivider uses bit-shifting and bitmasks in
 * order to divide the RGB values of a fed-in uint32_t
 * by a fed-in factor of 2.
 * 
 * ex. you call colorDivider(Color(255,255,255), 3)
 * This means it will do 255/(2^3), and return a
 * uint32_t equivalent to about (32,32,32)
 */
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

/*
 * colorWipe(), dualColorWipe(), and dualColorToneWipe() are
 * all early / Adafruit-included test functions intended to simply
 * test the light strand, they are not implemented currently in
 * any menu functions.
 */
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

/*
 * Early test function, lights up strip up to a
 * certain point based on the range, and turns off
 * the remaining part of the strip.
 */
void colorFollower(uint32_t c, int givenRange) {
  int i;
  int adjustedRange = givenRange/3;
  
  for (i=0; i < adjustedRange; i++) {
      strip.setPixelColor(i+4, c);
    }
  for (i=adjustedRange; i < strip.numPixels(); i++) {
      strip.setPixelColor(i+4, pixelOff);
    }  
  strip.show();
}

/*
 * Early iteration of "floating orb" idea, lights up
 * lights based around an object at a certain range
 * and nowhere else.
 */
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


uint32_t createColor(){
  uint32_t colorRange;
  int range = constrain(getRange2(), 0, 100);
  int rgbBase1 = map(range, 0, 20, 0, 255);
  int rgbBase2 = map(range, 20, 40, 0, 255);
  int rgbBase3 = map(range, 40, 60, 0, 255);
  int rgbBase4 = map(range, 60, 80, 0, 255);
  int rgbBase5 = map(range, 80, 100, 0, 255);
    
  if (range <= 20){
    colorRange = Color(rgbBase1, 0, 0);
  }
  else if (range <= 40){
    colorRange = Color(255, rgbBase2, 0);
  }
  else if (range <= 60){
    colorRange = Color(255-rgbBase3, 255, 0);
  }
  else if (range <= 80){
    colorRange = Color(0, 255, rgbBase4);
  } 
  else if(range <= 100){
    colorRange = Color(0, 255-rgbBase5, 255);
  }
  return colorRange;    
}

/*
 * proxGlow changes brightness of the entire strip
 * based on range of a single rangefinder.
 */
void proxGlow(uint32_t color) {
  m.lcd.clear();
  m.lcd.home();
  int glowRange;
  int i=0;
  while(1) {
    glowRange = getRange()*2;
    if(glowRange <= 10) {
      moddedColor = color;
    } else if(glowRange <= 25) {
      moddedColor = colorDivider(color,2);
    } else if(glowRange <= 50) {
      moddedColor = colorDivider(color,3);
    } else if(glowRange <= 75) {
      moddedColor = colorDivider(color,5);
    } else if(glowRange <= 100) {
      moddedColor = colorDivider(color,7);
    } else {
      moddedColor = pixelOff;
    }
    
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, moddedColor);
    }
    strip.show();
  }
}


/*
 * triRangeCircle() is supposed to use 3 rangefinders 
 * to make an "orb" of light follow your hands around
 * an LED strip arranged in a circle around the rangefinders.
 * TL;DR it reads the 3 rangefinders, disregards the "farthest"
 * rangefinder (one with the largest range value) and averages
 * the distance between the two closest to determine where
 * the light should be, and then also calculates the brightness
 * and what note to play. Works but takes practice to actually
 * control without a ton of flickering.
 */
void triRangeCircle(uint32_t color) {
  int range1;
  int range2;
  int range3;
  int averageRange;
  int LEDposition;
  uint32_t halfBrightness;
  uint32_t quarterBrightness;
  uint32_t eighthBrightness;
  int maxDist = 80;
  int root;
  
  while(1) {
    // get the 3 ranges
    range1 = getRange();
    range2 = getRange2();
    range3 = getRange3();

    /*
     * Determine which rangefinder is farthest and calculate
     * the average of the 2 remaining ranges.
     */
    if(range1 > range2 && range1 > range3) {
      averageRange = (range2 + range3)/2;
      root = map(constrain(range2, 0, 150), 0, 150, 0, 29);
    } else if(range2 > range1 && range2 > range3) {
      averageRange = (range1 + range3)/2;
      root = map(constrain(range3, 0, 150), 0, 150, 0, 29);
    } else if (range3 > range1 && range3 > range2) {
      averageRange = (range1 + range2)/2;
      root = map(constrain(range1, 0, 150), 0, 150, 0, 29);
    }
    // root is used to determine the note to be played

    /*
     *  moddedColor sets the brightness based on how close
     *  the object is to the two closest rangefinders.
     *  Basically, the previous function determines the
     *  position of the center of the light orb, and moddedColor
     *  determines the brightness of it (if both hands are close
     *  the light will be really bright, far will be dim.)
     */
    moddedColor = colorDivider(color, averageRange/10);
    quarterBrightness = colorDivider(moddedColor, 2);
    eighthBrightness = colorDivider(moddedColor, 4);
    /* 
     * Quarter and eighth brightness are created to be put 
     * next to the center light and create the "orb" appearance,
     * half brightness could be used but isn't because the
     * difference between full and half isn't very dramatic/visible.
     */

    /* 
     * where the rangefinders are, corresponding to the LEDS:
     *    /R1/ 3 4 5 6 7 8 9 /R2/ 10 11 12 13 14 15 16 17 /R3/ 18 19 20 21 22 23 24 
     *         |___________|      |_____________________|      |__________________|
     *               |                       |                           |
     *               7                       8                           7
     */
    

    /*
     * in the following function, there are several rangeX*[some number]
     * statements, the [some number] is determined by the number of LED's
     * in the third of the string covered by that rangefinder, as shown in
     * the above diagram. These take range of a single rangefinder and divide
     * it by the sum of the two ranges to get what fraction of the third that the
     * center should be at, then adds in an offset of 3 to account for having
     * 3 lights not actually in the circle, and then depending on what third
     * the light is in, adds in another offset to boost it up to the correct range.
     */
    if(range1 > range2 && range1 > range3) {
      LEDposition = ((range2*8)/(range2+range3))+3+7;
    } else if(range2 > range1 && range2 > range3) {
      LEDposition = ((range3*7)/(range3+range1))+3+15;
    } else if (range3 > range1 && range3 > range2) {
      LEDposition = ((range1*7)/(range1+range2))+3;
    }

    /*
     * if any 2 rangefinders are above maxDist, turn off all the lights, else
     * set the center light and set / check if the neighbor lights (quarterBrightness
     * and eighthBrightness) are over the end / beginning of the strand and
     * correct for it if necessary.
     */
    if((range1 > maxDist && range2 > maxDist) || (range2 > maxDist && range3 > maxDist) || (range1 > maxDist && range3 > maxDist)) {
      for(int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, pixelOff);
      }
    } else {
      if (LEDposition-2 < 3) {
        strip.setPixelColor(LEDposition-2+25, eighthBrightness);
      } else {strip.setPixelColor(LEDposition-2, eighthBrightness);}
      
      if (LEDposition-1 < 3) {
        strip.setPixelColor(LEDposition-1+25, quarterBrightness);
      } else {strip.setPixelColor(LEDposition-1, quarterBrightness);}\
      
      strip.setPixelColor(LEDposition, moddedColor);

      if (LEDposition+1 > 25) {
        strip.setPixelColor(LEDposition+1-25, quarterBrightness);
      } else {strip.setPixelColor(LEDposition+1, quarterBrightness);}

      if (LEDposition+2 > 25) {
        strip.setPixelColor(LEDposition+2-25, eighthBrightness);
      } else {strip.setPixelColor(LEDposition+2, eighthBrightness);}  
    }

    // turn on note, and show the contructed strand.
    noteOn(whiteKeys[root]);
    strip.show();
    delay(100); // longer delays prevent rapid flickering but also make it more finicky to control.

    /*
     * We immediately turn off those LED's we just turned on because
     * then we know we will have an empty string to write to again,
     * and as long as we don't strip.show() again, the LED's will just
     * maintain the previous values we sent to them until we show() again.
     */
    if (LEDposition-2 < 3) {
        strip.setPixelColor(LEDposition-2+25, pixelOff);
      } else {strip.setPixelColor(LEDposition-2, pixelOff);}
      
      if (LEDposition-1 < 3) {
        strip.setPixelColor(LEDposition-1+25, pixelOff);
      } else {strip.setPixelColor(LEDposition-1, pixelOff);}\
      
      strip.setPixelColor(LEDposition, pixelOff);

      if (LEDposition+1 > 25) {
        strip.setPixelColor(LEDposition+1-25, pixelOff);
      } else {strip.setPixelColor(LEDposition+1, pixelOff);}

      if (LEDposition+2 > 25) {
        strip.setPixelColor(LEDposition+2-25, pixelOff);
      } else {strip.setPixelColor(LEDposition+2, pixelOff);}  
  }
}


uint32_t rangeColor(){
  m.lcd.clear();
  uint32_t colorRange;
  while(1){
    int range = constrain(getRange(), 0, 50);
    int rgbBase1 = map(range, 0, 10, 0, 255);
    int rgbBase2 = map(range, 10, 20, 0, 255);
    int rgbBase3 = map(range, 20, 30, 0, 255);
    int rgbBase4 = map(range, 30, 40, 0, 255);
    int rgbBase5 = map(range, 40, 50, 0, 255);

    
    if (range <= 10){
      colorRange = Color(rgbBase1, 0, 0);
    }
    else if (range <= 20){
      colorRange = Color(255, rgbBase2, 0);
    }
    else if (range <= 30){
      colorRange = Color(255-rgbBase3, 255, 0);
    }
    else if (range <= 40){
      colorRange = Color(0, 255, rgbBase4);
    } 
    else if(range <= 50){
      colorRange = Color(0, 255-rgbBase5, 255);
    }
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, colorRange);
    }
    strip.show();
    delay(50);
    byte buttons = m.getButtons();
    if (buttons == BUTTON_LEFT){
      return;
    }
  }
}

/*
 * Wheel() is called by rainbow() to cycle the whole strip through a color wheel.
 */
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



//=============================== MIDI Functions ===============================


void noteOn(int pitch) {
  Serial.write(0x90);
  Serial.write(pitch);
  Serial.write(0x45);
}

void noteOff() {
  Serial.write(80);
}

int createRoot(){
  int root = map(constrain(getRange3(), 0, 150), 0, 150, 0, 29);
  return root;
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
  Serial.write(0x35);
  Serial.write(0x90);
  Serial.write(whiteKeys[root + 2]);
  Serial.write(0x35);
  Serial.write(0x90);
  Serial.write(whiteKeys[root + 4]);
  Serial.write(0x35);
}

//=========================== Miscellaneous Functions ==========================

// Functions for using 3 different ultrasonic rangefinders (same methods, just uses different pins):
int getRange() {
  digitalWrite(6, HIGH);
  delayMicroseconds(10);
  digitalWrite(6, LOW);
  int message_time = pulseIn(7, HIGH);
  int range = message_time/58.0;
  return range;
}

int getRange2() {
  digitalWrite(9, HIGH);
  delayMicroseconds(10);
  digitalWrite(9, LOW);
  int message_time = pulseIn(10, HIGH);
  int range = message_time/58.0;
  return range;
}

int getRange3() {
  digitalWrite(11, HIGH);
  delayMicroseconds(10);
  digitalWrite(11, LOW);
  int message_time = pulseIn(12, HIGH);
  int range = message_time/58.0;
  return range;
}

/*
 * The simplePower function was made because early in our
 * project we thought actually calculating exponents was
 * making our functions slow, but this may actually be
 * completely unnecessary.
 */
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
