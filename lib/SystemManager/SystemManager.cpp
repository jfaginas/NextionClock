#include "SystemManager.h"

SystemManager::SystemManager()
    : nextion(Serial2)
{}

void SystemManager::begin() {
    rtc.begin();
    scheduler.begin();
    nextion.begin(9600);

    if (rtc.isPowerLost()) {
        Serial.println("[RTC] Pérdida de energía detectada. Mostrar pantalla de configuración de fecha.");
        nextion.gotoPage("page_SetDate");
    } else {
        DateTime now;
        if (rtc.getDateTime(now)) {
            nextion.gotoPage("page_Init");
            nextion.showDateTime(now);
            nextion.showWeekday(now.weekday);
        } else {
            Serial.println("[RTC] Fallo al leer la hora. Ir a pantalla de configuración.");
            nextion.gotoPage("page_SetDate");
        }
    }

    Serial.println("SystemManager iniciado.");
}

void SystemManager::update() {
    nextion.update();

    static unsigned long lastDisplayUpdate = 0;
    unsigned long now = millis();

    if (now - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = now;

        DateTime nowTime;
        if (rtc.getDateTime(nowTime)) {

            Serial.printf("Fecha actual: %s (%s)\n",
              nowTime.toDateString().c_str(),
              nextion.weekdayToString(nowTime.weekday));

            nextion.showDateTime(nowTime);
            nextion.showWeekday(nowTime.weekday);
            scheduler.update(nowTime, true);  // Flag habilitación fija por ahora
        }
    }
    if (nextion.isCommandAvailable()) {
        String cmd = nextion.getLastCommand();
        Serial.print("[System] Comando recibido: ");
        Serial.println(cmd);
        handleCommand(cmd);
    }
    // if (nextion.isCommandAvailable()) {
    //     String cmd = nextion.getLastCommand();
    //     handleCommand(cmd);
    // }
}

// void SystemManager::handleCommand(const String& cmd) {
//     if (cmd.startsWith("SETDATE=")) {
//         handleSetDate(cmd.substring(8));
//     } else if (cmd.startsWith("SETTIME=")) {
//         handleSetTime(cmd.substring(8));
//     } else if (cmd.startsWith("SCHED=")) {
//         scheduler.handleSchedulerCommand(cmd.substring(6));

//         Serial.print("[System] Comando de programación recibido: "); //
//         Serial.println(cmd.substring(6));                            //

//     } else {
//         Serial.println("[Nextion] Comando desconocido: " + cmd);
//     }
// }

void SystemManager::handleCommand(const String& cmd) {
    if (cmd.startsWith("SETDATE=")) {
        handleSetDate(cmd.substring(8));
    } else if (cmd.startsWith("SETTIME=")) {
        handleSetTime(cmd.substring(8));
    } else if (cmd.startsWith("SCHED=")) {
        scheduler.handleSchedulerCommand(cmd.substring(6));
        Serial.print("[System] Comando de programación recibido: ");
        Serial.println(cmd.substring(6));
    } else if (cmd == "CLEAR") {
        Serial.println("[System] Comando recibido: CLEAR");
        scheduler.clearSchedule();  // Llama a borrar todos los slots
    } else if (cmd.startsWith("SHOW=")) {
        int day = cmd.substring(5).toInt();  // 0 a 6
        Serial.print("[System] Comando recibido: SHOW=");
        Serial.println(day);
        scheduler.showScheduleForDay(day,nextion);
    } else {
        Serial.println("[Nextion] Comando desconocido: " + cmd);
    }
}

void SystemManager::handleSetDate(const String& data) {
    int d, m, y;
    if (sscanf(data.c_str(), "%d,%d,%d", &d, &m, &y) == 3) {
        if (isValidDate(d, m, y)) {
            tempDate.day = d;
            tempDate.month = m;
            tempDate.year = y;
            tempDate.weekday = calculateWeekday(tempDate.day, tempDate.month, tempDate.year);
            Serial.printf("Calculado: weekday = %d\n", tempDate.weekday);
            timeSetStepPending = true;
            nextion.gotoPage("page_SetTime");
        } else {
            nextion.showError("Fecha inválida");
        }
    } else {
        nextion.showError("Error de formato");
    }
}

void SystemManager::handleSetTime(const String& data) {
    if (!timeSetStepPending) {
        nextion.showError("Primero setear fecha");
        return;
    }

    int h, m;
    if (sscanf(data.c_str(), "%d,%d", &h, &m) == 2 && h < 24 && m < 60) {
        tempDate.hour = h;
        tempDate.minute = m;
        tempDate.second = 0;

        if (rtc.setDateTime(tempDate)) {
            nextion.gotoPage("page_Init");
            nextion.showDateTime(tempDate);
            nextion.showWeekday(tempDate.weekday);
            timeSetStepPending = false;
            Serial.println("[RTC] Fecha y hora actualizadas correctamente.");
        } else {
            nextion.showError("Falló setTime");
        }
    } else {
        nextion.showError("Hora inválida");
    }
}



