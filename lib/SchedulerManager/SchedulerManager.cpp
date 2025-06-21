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

    TimePoint current = { now.hour, now.minute };

    // Día actual
    uint8_t indexToday = (now.weekday + 5) % 7;
    const DailySchedule& today = schedule.days[indexToday];

    // Día anterior
    uint8_t indexYesterday = (indexToday + 6) % 7;  // día anterior
    const DailySchedule& yesterday = schedule.days[indexYesterday];

    // Ver slots del día actual
    for (const auto& slot : today.slots) {
        if (!slot.enabled) continue;
        if (isWithinInterval(current, slot.onTime, slot.offTime)) {
            Serial.printf("[Scheduler] ACTUAL: %02u:%02u → %02u:%02u\n",
                          slot.onTime.hour, slot.onTime.minute,
                          slot.offTime.hour, slot.offTime.minute);
            ledState = true;
            return;
        }
    }

    // Ver slots del día anterior que cruzan medianoche
    for (const auto& slot : yesterday.slots) {
        if (!slot.enabled) continue;
        uint16_t onMin  = slot.onTime.hour * 60 + slot.onTime.minute;
        uint16_t offMin = slot.offTime.hour * 60 + slot.offTime.minute;
        if (onMin > offMin) {  // cruza medianoche
            if (isWithinInterval(current, slot.onTime, slot.offTime)) {
                Serial.printf("[Scheduler] AYER (overnight): %02u:%02u → %02u:%02u\n",
                              slot.onTime.hour, slot.onTime.minute,
                              slot.offTime.hour, slot.offTime.minute);
                ledState = true;
                return;
            }
        }
    }
}

bool SchedulerManager::isWithinInterval(const TimePoint& now, const TimePoint& start, const TimePoint& end) {
    uint16_t nowMin   = now.hour * 60 + now.minute;
    uint16_t startMin = start.hour * 60 + start.minute;
    uint16_t endMin   = end.hour * 60 + end.minute;

    if (startMin < endMin) {
        // Caso normal: dentro del mismo día
        return nowMin >= startMin && nowMin < endMin;
    } else if (startMin > endMin) {
        // Caso overnight: el intervalo cruza medianoche
        return nowMin >= startMin || nowMin < endMin;
    } else {
        // Caso especial: start == end → sin duración
        return false;
    }
}

void SchedulerManager::setSchedule(uint8_t day, uint8_t slot, const TimePoint& on, const TimePoint& off, bool enabled) {
    Serial.printf("[DEBUG] setSchedule() → día=%u, slot=%u, ON=%02u:%02u, OFF=%02u:%02u, enab=%d\n",
    day, slot, on.hour, on.minute, off.hour, off.minute, enabled);

    if (day > 6 || slot > 1) return;
    schedule.days[day].slots[slot] = { on, off, (uint8_t)enabled };
    Serial.println("[DEBUG] Llamando a saveToEEPROM()");
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
    Serial.println("[DEBUG] Entrando en saveToEEPROM()");
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

    Serial.println("[DEBUG] Entrando en handleSchedulerCommand()");
    
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

        Serial.println("[DEBUG] Llamado a setSchedule(), debería grabar en EEPROM");

        Serial.printf("[Scheduler] Guardado: día %u slot %u %02u:%02u → %02u:%02u (%s)\n",
            day, slot, on.hour, on.minute, off.hour, off.minute,
            enabled ? "ON" : "OFF");
    } else {
        Serial.println("[Scheduler] Error de formato en comando SCHED=");
    }
}

void SchedulerManager::clearSchedule() {
    for (auto& day : schedule.days) {
        for (auto& slot : day.slots) {
            slot.onTime = {0, 0};
            slot.offTime = {0, 0};
            slot.enabled = false;
        }
    }
    saveToEEPROM();
    Serial.println("[Scheduler] Programación borrada.");
}

void SchedulerManager::showScheduleForDay(uint8_t day, NextionManager& display) {

    if (day > 6) return;

    for (int s = 0; s < 2; ++s) {
        const ScheduleSlot& slot = schedule.days[day].slots[s];

        char buf[32];
        if (slot.enabled) {
            snprintf(buf, sizeof(buf), "%02u:%02u - %02u:%02u", 
                     slot.onTime.hour, slot.onTime.minute,
                     slot.offTime.hour, slot.offTime.minute);
        } else {
            snprintf(buf, sizeof(buf), "Vacio");
        }

        String cmd = "slot" + String(s) + ".txt=\"" + String(buf) + "\"";
        display.sendCommand(cmd);  // envía comando al Nextion
    }
}