# Nextion Clock – ESP32 + DS3231 + EEPROM + Nextion

Firmware modular y robusto para un sistema de programación horaria semanal con visualización e interacción mediante una pantalla táctil Nextion.

---

## 📌 Descripción general

El proyecto **Nextion Clock** permite controlar salidas (por ejemplo, relés o LEDs) según horarios semanales definidos por el usuario. El sistema:

- Muestra la fecha y hora actual en una pantalla Nextion.
- Permite modificar fecha y hora desde la interfaz táctil.
- Soporta programación de encendidos/apagados diarios (hasta 2 ciclos por día).
- Almacena los datos en una memoria EEPROM externa (24C32).
- Mantiene la hora mediante un RTC DS3231, incluso sin energía.
- Ejecuta los ciclos aún si se reinicia el ESP32.
- Permite borrar toda la programación desde la interfaz.
- Permite consultar los ciclos configurados por día.

---

## 🧱 Arquitectura

- **Microcontrolador:** ESP32
- **RTC:** DS3231
- **EEPROM externa:** 24C32 (I2C)
- **Pantalla táctil:** Nextion (modelo básico)
- **IDE:** VS Code + PlatformIO
- **Lenguaje:** C++ (orientado a objetos, modular)

---
## 🔌 Conexiones de hardware

| Componente          | Pin ESP32       | Detalles                                 |
|---------------------|------------------|-------------------------------------------|
| **DS3231 (RTC)**    | GPIO21 (SDA)     | Bus I2C compartido                        |
|                     | GPIO22 (SCL)     | Bus I2C compartido                        |
| **EEPROM 24C32**    | GPIO21 (SDA)     | Misma línea I2C que el DS3231             |
|                     | GPIO22 (SCL)     | Misma línea I2C que el DS3231             |
|                     | VCC              | 3.3V o 5V según módulo                    |
|                     | GND              | GND común                                |
| **Nextion Display** | GPIO17 (TX2)     | TX del ESP32 → RX del Nextion             |
|                     | GPIO16 (RX2)     | RX del ESP32 ← TX del Nextion             |
|                     | VCC              | 5V (o 3.3V si el modelo lo soporta)       |
|                     | GND              | GND común                                |
| **LED controlado**  | GPIO2            | Salida digital controlada por horario     |
---
## 💡 Consideraciones

- El bus I2C (RTC y EEPROM) puede tener resistencias de pull-up en el módulo o ser añadidas externamente (4.7k – 10k Ω típicas).

- El display Nextion se comunica vía UART2 (Serial2). Asegurate de no usar esos pines para otras funciones.

- GPIO2 es usado como salida de control (enciende el led interno al módulo ESP32). Puede conectarse a un LED con resistencia limitadora o a un módulo de relé.

---
## 📂 Estructura del proyecto

```text
NextionClock/
├── src/
│   └── main.cpp  ← Programa principal (setup + loop)
│
├── lib/
│   ├── SystemManager/
│   │   ├── SystemManager.h
│   │   └── SystemManager.cpp
│   │
│   ├── DS3231Manager/
│   │   ├── DS3231Manager.h
│   │   └── DS3231Manager.cpp
│   │
│   ├── EEPROMManager/
│   │   ├── EEPROMManager.h
│   │   └── EEPROMManager.cpp
│   │
│   ├── SchedulerManager/
│   │   ├── SchedulerManager.h
│   │   └── SchedulerManager.cpp
│   │
│   ├── NextionManager/
│   │   ├── NextionManager.h
│   │   └── NextionManager.cpp
│   │
│   └── DateTimeUtils/
│       ├── DateTimeUtils.h
│       └── DateTimeUtils.cpp
│
├── extras/
│   ├── NextionClock.hmi              ← Archivo fuente editable con Nextion Editor
│   └── NextionClock.tft              ← Archivo compilado para la pantalla Nextion
│
├── platformio.ini  ← Configuración de plataforma, velocidad, build flags, etc.
└── README.md                         ← Documentación principal del proyecto
```
## 🧱 Diseño modular

Los módulos del sistema ubicados dentro de la carpeta `lib/` están desarrollados siguiendo el principio de responsabilidad única (*Single Responsibility Principle*).  
Cada uno encapsula una funcionalidad específica (como gestión del RTC, comunicación con el display, acceso a la EEPROM, etc.), lo que permite un código más limpio, mantenible y reutilizable.


---
## 📦 Descripción de cada módulo del sistema

### 🔧 `SystemManager`

Coordinador principal del sistema.  
Inicializa todos los módulos y delega tareas entre ellos.  
Se encarga de recibir, interpretar y redirigir los comandos provenientes del display Nextion.

---

### 🕒 `DS3231Manager`

Encapsula el manejo del RTC DS3231.  
Permite obtener y configurar la fecha y hora actual, incluyendo el día de la semana.  
Internamente gestiona la conversión BCD y la comunicación I2C.

---

### 💾 `EEPROMManager`

Módulo que controla la memoria EEPROM externa 24C32.  
Permite almacenar estructuras de datos (como la programación horaria) de forma persistente.  
Abstrae la comunicación I2C y ofrece métodos de lectura y escritura tipo bloque.

---

### 📺 `NextionManager`

Controlador de la pantalla táctil Nextion.  
Se encarga de enviar comandos (por ejemplo, para actualizar campos de texto) y recibir eventos generados por el usuario.  
Incluye manejo de buffers y conversión de valores como el día de la semana a texto.

---

### 📆 `SchedulerManager`

Gestiona la programación horaria semanal del sistema.  
Permite definir hasta dos rangos de encendido/apagado por día, para cada día de la semana.  
Evalúa si el LED debe estar encendido en base a la hora actual y los slots programados.  
Contempla intervalos que cruzan la medianoche y permite consultar o borrar la programación almacenada.

---

### ⏱️ `DateTimeUtils`

Contiene funciones auxiliares para trabajar con estructuras de fecha y hora (`TimePoint`, `DateTime`).  
Incluye funciones para comparar horarios, convertir días de la semana y validar intervalos.

---

> 💡 Insisto -> todos los módulos están escritos con separación clara de responsabilidades y pueden reutilizarse o extenderse en futuros proyectos.


---

## 🛠️ Funcionalidad disponible

| Función                  | Método de acceso           |
|--------------------------|----------------------------|
| Ver hora/fecha actual    | Pantalla Nextion           |
| Setear hora y fecha      | Botón en pantalla          |
| Programar horarios       | `SCHED=` desde Nextion     |
| Consultar programación   | `SHOW=` desde Nextion      |
| Borrar toda la memoria   | Botón `CLEAR` en pantalla  |
| Encender/apagar salida   | Automático por programación |

---

## 📡 Comunicación

- **ESP32 ↔ Nextion:** Serial (UART2)
- **ESP32 ↔ RTC/EEPROM:** I2C

---

## 📋 Requisitos

- VS Code + PlatformIO
- Fuente `NextionClock.hmi` abierta con Nextion Editor (Windows)
- Carga del `NextionClock.tft` en pantalla mediante tarjeta microSD o USB-TTL

---
📦 Consideraciones

- El firmware fue diseñado para ser modular, claro y mantenible.
- La lógica que evalúa los encendidos/apagados contempla intervalos que cruzan la medianoche.
- Se implementaron logs en la terminal para facilitar el debug durante el desarrollo.

---
📌 Estado actual

    ✅ Firmware funcional
    ✅ Persistencia de datos en EEPROM
    ✅ Sincronización RTC
    ✅ Interfaz Nextion operativa
    🔜 Futuras mejoras: múltiples salidas.

📖 Licencia

Este proyecto está licenciado bajo la licencia MIT.

👤 Autor

Es un ejercicio educativo desarrollado por José Faginas, usando el siguiente toolchain: VsCode + PlatformIO en C++ y para la interfaz de usuario el Nextion Editor. 



