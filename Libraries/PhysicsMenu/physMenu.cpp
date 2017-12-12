#include "physMenu.h"


void PhysicsMenu::dispatch() {
    byte buttons = getDebouncedButtons(false);          // 'buttons' value depends on what was presssed
    if (buttons) {                              // Anything but but zero does next section
        if (buttons == BUTTON_UP) {
            this->opt++;   // Up button increments menu option
            refresh = true;
        }
        if (buttons == BUTTON_DOWN) {
            this->opt--;   // Down button decrements menu option
            refresh = true;
        }
        if (this->opt >= this->nextItem) {
            this->opt = 0;   // rollover menu options
        }
        if (this->opt < 0)   {
            this->opt = this->nextItem -1;   // rollunder menu options
        }
        if (buttons & BUTTON_SELECT) {
            this->items[opt].func();
            refresh = true;
        }
    }

    if(refresh) {
        this->lcd.clear();
        this->lcd.home();
        if(this->nextItem == 0) {
            this->lcd.print("Empty PhysicsMenu");
        } else {
            this->lcd.print(this->items[this->opt].name);
        }
        refresh = false;
    }

}

byte PhysicsMenu::getDebouncedButtons(bool lock) {
    byte readval = lcd.readButtons();
    if(readval != this->lastBState) {
        this->lockTimer = millis();
        this->debounceTimer = millis();
    }
    if((millis() - this->debounceTimer) > this->debounceThreshold) {
        if(lock) {
            if(readval != bState) {
                this->bState = readval;
                lockTimer = millis();
                return this->bState;
            } else if((millis() - lockTimer ) > lockThreshold)
                return this->bState;
            return 0;
        } else if(readval != this->bState) {
            this->lastBState = this->bState = readval;
            return this->bState;
        }
    }
    this->lastBState = readval;
    return 0;
}

void PhysicsMenu::addItem(char *name, funcPtr func) {

    if(this->nextItem >= this->size) {
        Serial.println("Can't, srry :(");
        return;
    }
    this->items[this->nextItem].name = name;
    this->items[this->nextItem].func = func;
    this->nextItem++;
}

void PhysicsMenu::begin() {
    lcd = Adafruit_RGBLCDShield();  // we call library functions using 'lcd'
    lcd.begin(16, 2);  // set up the LCD's number of columns and rows:
    lcd.setBacklight(6);  // Red=1, Yellow=3, Green=2, Teal=6, Blue=4, Violet=5, White
}

void PhysicsMenu::setMessage(int msg){
    setMessage(String(msg));
}
void PhysicsMenu::setMessage(String msg){
    lcd.clear();
    lcd.home();
    lcd.print(msg);
}

void PhysicsMenu::appendMessage(int msg){ appendMessage(String(msg)); }
void PhysicsMenu::appendMessage(String msg){ lcd.print(msg); }
