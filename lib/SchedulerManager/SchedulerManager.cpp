#include "SchedulerManager.h"

#define EEPROM_SCHEDULE_SIZE sizeof(WeeklySchedule)

SchedulerManager::SchedulerManager() : ledState(false) {}

void SchedulerManager::begin() {
    pinMode(SCHEDULER_LED_PIN, OUTPUT);
    loadFromEEPROM();
    Serial.printf("Tamaño de WeeklySchedule = %u bytes\n", sizeof(WeeklySchedule));

}

void SchedulerManager::update(const DateTime& now, bool enableFlag) {
    applySchedule(now, enableFlag);
    digitalWrite(SCHEDULER_LED_PIN, ledState ? HIGH : LOW);
}

void SchedulerManager::applySchedule(const DateTime& now, bool enableFlag) {
    ledState = false;
    if (!enableFlag) return;

    uint8_t index = now.weekday == 0 ? 6 : now.weekday - 1;  // 0=Lun, ..., 6=Dom
    const DailySchedule& day = schedule.days[index];
    TimePoint current = { now.hour, now.minute };

    for (const auto& slot : day.slots) {
        if (!slot.enabled) continue;
        if (isWithinInterval(current, slot.onTime, slot.offTime)) {
            ledState = true;
            break;
        }
    }
}

bool SchedulerManager::isWithinInterval(const TimePoint& now, const TimePoint& on, const TimePoint& off) {
    if (on < off) {
        return (on <= now && now < off);
    } else {  // Intervalo cruzando medianoche
        return (now >= on || now < off);
    }
}

void SchedulerManager::setSchedule(uint8_t day, uint8_t slot, const TimePoint& on, const TimePoint& off, bool enabled) {
    if (day > 6 || slot > 1) return;
    schedule.days[day].slots[slot] = { on, off, (uint8_t)enabled };
    saveToEEPROM();
}

const WeeklySchedule& SchedulerManager::getSchedule() const {
    return schedule;
}

void SchedulerManager::loadFromEEPROM() {
    EEPROMManager eeprom(EEPROM_ADDRESS);
    eeprom.begin();
    eeprom.readBytes(EEPROM_SCHEDULE_ADDR, (uint8_t*)&schedule, EEPROM_SCHEDULE_SIZE);

    Serial.println("[Scheduler] Horarios cargados desde EEPROM:");
    for (int d = 0; d < 7; ++d) {
        for (int s = 0; s < 2; ++s) {
            const auto& slot = schedule.days[d].slots[s];
            Serial.printf("Día %d Slot %d: %02u:%02u → %02u:%02u (%s)\n",
                d, s,
                slot.onTime.hour, slot.onTime.minute,
                slot.offTime.hour, slot.offTime.minute,
                slot.enabled ? "ON" : "OFF");
        }
    }
}

void SchedulerManager::saveToEEPROM() {
    EEPROMManager eeprom(EEPROM_ADDRESS);
    eeprom.begin();
    eeprom.writeBytes(EEPROM_SCHEDULE_ADDR, (uint8_t*)&schedule, EEPROM_SCHEDULE_SIZE);
}

void SchedulerManager::handleSchedulerCommand(const String& cmd) {
    int values[7];
    int count = 0;

    char buffer[cmd.length() + 1];
    cmd.toCharArray(buffer, sizeof(buffer));
    char* token = strtok(buffer, ",");

    while (token && count < 7) {
        values[count++] = atoi(token);
        token = strtok(nullptr, ",");
    }

    if (count == 7) {
        uint8_t day = values[0];
        uint8_t slot = values[1];
        TimePoint on = { (uint8_t)values[2], (uint8_t)values[3] };
        TimePoint off = { (uint8_t)values[4], (uint8_t)values[5] };
        bool enabled = values[6];

        setSchedule(day, slot, on, off, enabled);

        Serial.printf("[Scheduler] Guardado: día %u slot %u %02u:%02u → %02u:%02u (%s)\n",
            day, slot, on.hour, on.minute, off.hour, off.minute,
            enabled ? "ON" : "OFF");
    } else {
        Serial.println("[Scheduler] Error de formato en comando SCHED=");
    }
}
