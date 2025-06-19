#pragma once

#include <Arduino.h>
#include <DateTimeUtils.h>

class NextionManager {
public:
    explicit NextionManager(HardwareSerial& serial);
    void begin(uint32_t baudrate = 9600);
    void update();

    void showDateTime(const DateTime& dt);
    void showWeekday(uint8_t weekday);
    void showError(const String& msg);
    void gotoPage(const String& pageName);

    bool isCommandAvailable() const;
    String getLastCommand();
    const char* weekdayToString(uint8_t weekday);

private:
    HardwareSerial& _serial;
    String _buffer;
    String _lastCommand;
    bool _commandReady;

    void parseSerial();
    void sendCommand(const String& cmd);
    // const char* weekdayToString(uint8_t weekday);
};

