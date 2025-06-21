#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <EEPROMManager.h>
#include <DateTimeUtils.h>
#include <NextionManager.h>

#define SCHEDULER_LED_PIN 2
#define EEPROM_ADDRESS 0x57
#define EEPROM_SCHEDULE_ADDR 0x0000

struct __attribute__((packed)) TimePoint {
    uint8_t hour;
    uint8_t minute;
};

inline bool operator==(const TimePoint& a, const TimePoint& b) {
    return a.hour == b.hour && a.minute == b.minute;
}

inline bool operator<(const TimePoint& a, const TimePoint& b) {
    return (a.hour < b.hour) || (a.hour == b.hour && a.minute < b.minute);
}

inline bool operator!=(const TimePoint& a, const TimePoint& b) { return !(a == b); }
inline bool operator>(const TimePoint& a, const TimePoint& b) { return b < a; }
inline bool operator<=(const TimePoint& a, const TimePoint& b) { return !(b < a); }
inline bool operator>=(const TimePoint& a, const TimePoint& b) { return !(a < b); }

struct __attribute__((packed)) ScheduleSlot {
    TimePoint onTime;
    TimePoint offTime;
    uint8_t enabled;  // bool de 8 bits
};

struct __attribute__((packed)) DailySchedule {
    ScheduleSlot slots[2];
};

struct __attribute__((packed)) WeeklySchedule {
    DailySchedule days[7];  // 0=Lunes ... 6=Domingo
};

class SchedulerManager {
public:
    SchedulerManager();
    void begin();
    void update(const DateTime& now, bool enableFlag);
    void setSchedule(uint8_t day, uint8_t slot, const TimePoint& on, const TimePoint& off, bool enabled);
    const WeeklySchedule& getSchedule() const;

    void handleSchedulerCommand(const String& cmd);  // Nuevo
    void clearSchedule();
    //void showScheduleForDay(uint8_t day);
    void showScheduleForDay(uint8_t day, NextionManager& display);

private:
    WeeklySchedule schedule;
    bool ledState;

    void applySchedule(const DateTime& now, bool enableFlag);
    bool isWithinInterval(const TimePoint& now, const TimePoint& on, const TimePoint& off);

    void loadFromEEPROM();
    void saveToEEPROM();
};
