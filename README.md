🎯 Objetivo principal del sistema

Permitir al usuario configurar fecha y hora en un módulo RTC DS3231, visualizar esa información en un display Nextion, y controlar un LED conectado al ESP32 en base a una programación semanal de horarios ON/OFF. Todo el sistema guarda su configuración de forma persistente en una memoria EEPROM externa (24C32) conectada por I2C.
🧩 Componentes físicos utilizados
Componente	Función	Detalles
ESP32	Microcontrolador principal	UART2 con Nextion, I2C con RTC y EEPROM
DS3231	Reloj de tiempo real (RTC)	Dirección I2C: 0x68
EEPROM 24C32	Almacenamiento persistente	Dirección I2C: 0x57, 16 bits, 4KBytes
Display Nextion	Interfaz táctil para el usuario	Conectado a Serial2
LED GPIO2	Salida controlada por programación	Encendido según horario y flag
⚙️ Flujo general del sistema
🔌 Al iniciar el sistema (setup)

    Se inicializan:

        Bus I2C (Wire)

        Puerto serial Serial2 para el Nextion

        RTC DS3231

        EEPROM 24C32

        Horarios cargados desde la EEPROM (WeeklySchedule)

    Se verifica si el RTC perdió la hora (bit OSF):

        ✔ Si perdió la hora → se muestra la pantalla page_configDate.

        ✔ Si el RTC tiene fecha válida → se muestra page_Init con fecha y hora actuales.

🖥️ Pantalla principal (page_Init)

    Se actualiza cada 1 segundo:

        t_date.txt: fecha (formato DD/MM/AAAA)

        t_hour.txt: hora (formato HH:MM)

        t_ddls.txt: día de la semana (ej. "Martes")

    El usuario puede acceder a un menú de configuración (botón bt_config, a implementar).

📅 Configuración manual de fecha y hora

    En page_configDate:

        El usuario elige día, mes, año.

        Al presionar bt_updateDate, se envía:

    SETDATE=dd,mm,aaaa

    El firmware valida la fecha:

        Si válida → cambia a page_Time.

        Si inválida → muestra error en t_error.txt.

En page_Time:

    El usuario elige hora y minutos.

    Al presionar bt_updateTime, se envía:

        SETTIME=hh,mm

        El firmware completa el objeto DateTime con los datos anteriores y actualiza el RTC.

🕹 Programación de encendido/apagado (función activa)

    Se programa mediante comandos del Nextion como:

SCHED=2,0,08,30,10,00,1

Interpreta como:
Campo	Significado
2	Día (2 = miércoles si 0=lunes)
0	Slot (0 o 1)
08,30	Hora de encendido
10,00	Hora de apagado
1	Habilitado

    Se permiten 2 slots diarios por día.

    Los datos se guardan automáticamente en la EEPROM (0x57) en formato binario (WeeklySchedule = 70 bytes).

    En cada segundo del loop, el sistema:

        Consulta la hora actual.

        Evalúa si el LED GPIO2 debe estar encendido según los slots habilitados y el flag de habilitación (por ahora true fijo).

💾 Persistencia de datos
Datos persistidos	Dónde se guardan	Cuándo
Horarios ON/OFF	EEPROM 24C32 (0x57)	Cada vez que se recibe SCHED=
Fecha/Hora RTC	Interno al DS3231	Al finalizar SETTIME=...
(Futuro) backup	EEPROM opcional	Podría agregarse si se desea
⚠️ Protección contra pérdida de energía

    Si el RTC pierde la hora (falta batería), se detecta al arranque mediante isPowerLost().

    Se fuerza al usuario a ingresar una nueva fecha/hora para evitar funcionamiento incorrecto.

📦 Estructura modular del firmware
Módulo	Función
main.cpp	Cuerpo mínimo que instancia SystemManager
SystemManager	Orquesta todo el sistema (setup, loop, interacción con display y RTC)
NextionManager	Interfaz básica con el display Nextion (UART2)
DS3231Manager	Manejo del RTC (lectura/escritura, detección de power loss)
DateTimeUtils	Validación de fecha, string helpers
SchedulerManager	Maneja los slots semanales, control del LED, guardado/lectura desde EEPROM
EEPROMManager	Encapsula operaciones de lectura y escritura con 24C32 vía I2C
🧠 Mejoras posibles a futuro (Etapas 4+)

    ✅ Agregar checksum o firma para validar que la EEPROM contiene datos consistentes.

    ✅ Guardar copia de respaldo de DateTime en EEPROM.

    ✅ Permitir activar/desactivar todo el sistema mediante una entrada digital (flag).

    ✅ Implementar interfaz visual para gestionar SCHED= desde la pantalla táctil.

    ✅ Integrar librería Itead original de Nextion con NexButton, NexText, etc.


