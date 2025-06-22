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

## 📂 Estructura del proyecto

```text
NextionClock/
├── src/
│   └── main.cpp                      ← Programa principal (setup + loop)
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
├── .pio/                             ← Carpeta generada por PlatformIO (build system)
│   └── (no se incluye en el control de versiones)
│
├── .vscode/                          ← Configuración de VSCode (tasks, launch, etc.)
│   └── (opcional)
│
├── .gitignore                        ← Archivos a excluir en Git
├── platformio.ini                    ← Configuración de plataforma, velocidad, build flags, etc.
└── README.md                         ← Documentación principal del proyecto

```
---
## 📦 Módulos del sistema

El proyecto está diseñado con una arquitectura modular y orientada a objetos.  
A continuación se describen brevemente los componentes principales:

---

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

> 💡 Todos los módulos están escritos con separación clara de responsabilidades y pueden reutilizarse o extenderse en futuros proyectos.


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



