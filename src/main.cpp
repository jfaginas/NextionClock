#include <Arduino.h>
#include "SystemManager.h"

SystemManager controller;

void setup() {
    Serial.begin(115200);
    controller.begin();
}

void loop() {
    controller.update();
}
