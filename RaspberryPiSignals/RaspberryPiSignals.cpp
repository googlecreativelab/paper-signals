/*
Copyright 2017 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
*/
#include <iostream>
#include "APICalls.h"

// Physical Pins.
#define ACTION_LED_PIN 13

PaperSignals mySignal;

void setupIndicatorLEDs()
{
    pinMode(ACTION_LED_PIN, OUTPUT);
}

void SetActionLEDOn()
{
    digitalWrite(ACTION_LED_PIN, false);
}

void SetActionLEDOff()
{
   digitalWrite(ACTION_LED_PIN, true); 
}

void setup() {

    setupIndicatorLEDs();
    SetActionLEDOn();    

    std::cout << "Starting" << std::endl;

    mySignal.StartUp();
}

void loop() {
    SetActionLEDOn();

    mySignal.RunPaperSignals();

    SetActionLEDOff();

    delay(5000);
}

int main(){
    wiringPiSetupPhys();

    setup();
    while(true){
        loop();
    }
}


