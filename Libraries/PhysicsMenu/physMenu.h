#ifndef PHYS304_MENU
#define PHYS304_MENU
#define MAX_SIZE 10
#include <Arduino.h>
#include <Wire.h>  // For the I2C communication used by LCD shield 
#include <Adafruit_RGBLCDShield.h>  // Library functions for the

typedef void (*funcPtr)();

typedef struct {
    char *name;
    funcPtr func;
} funcNamePair;

class PhysicsMenu {

private:
    funcNamePair items[MAX_SIZE];
    int size;
    int nextItem;
    int opt;
    bool refresh = true;
    long debounceTimer = 0;
    long lockTimer = 0;
    long debounceThreshold = 50;
    long lockThreshold = 300;
    byte lastBState = 0;
    byte bState = 0;

public:
    void dispatch();

    void addItem(char *name, funcPtr func);
    void begin();
    Adafruit_RGBLCDShield lcd;

    void setMessage(String msg);
    void setMessage(int msg);
    void appendMessage(String msg);
    void appendMessage(int msg);

    byte getDebouncedButtons(bool lock);
    byte getButtons(){ lockTimer = millis(); lastBState = bState = lcd.readButtons(); return bState; }

    PhysicsMenu() {
        nextItem = 0;
        opt = 0;
        size = MAX_SIZE;
    }

    ~PhysicsMenu() {}
};

#endif

