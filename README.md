# PRACTICA-2
# Práctica 2: Interrupciones en ESP32

## Introducción
En esta práctica se estudian las interrupciones en el microcontrolador ESP32 mediante dos enfoques:

- **Práctica A:** Interrupciones por GPIO.
- **Práctica B:** Interrupciones por temporizador (Timer).

Se utilizarán un botón y dos LEDs para visualizar el efecto de las interrupciones.

## 1.- Práctica A: Interrupción por GPIO

### Código:
```cpp
struct Button {
const uint8_t PIN;
uint32_t numberKeyPresses;
bool pressed;
};
Button button1 = {18, 0, false};
void IRAM_ATTR isr() {
button1.numberKeyPresses += 1;
button1.pressed = true;
}
void setup() {
Serial.begin(115200);
pinMode(button1.PIN, INPUT_PULLUP);
attachInterrupt(button1.PIN, isr, FALLING);
}
void loop() {
if (button1.pressed) {
Serial.printf("Button 1 has been pressed %u times\n",
button1.numberKeyPresses);
button1.pressed = false;
}

//Detach Interrupt after 1 Minute
static uint32_t lastMillis = 0;
if (millis() - lastMillis > 60000) {
lastMillis = millis();
detachInterrupt(button1.PIN);
Serial.println("Interrupt Detached!");
}
}
```

### Funcionamiento
Cuando se presiona el botón, se activa una interrupción que ejecuta la función isr(), la cual incrementa el contador de pulsaciones y marca el botón como presionado.
En el bucle principal (loop()), el código verifica si el botón ha sido pulsado y muestra en el monitor serie la cantidad de veces que ha ocurrido.
Si transcurre un minuto sin nuevas interrupciones, la función se deshabilita automáticamente.

#### Salida esperada en el puerto serie:
```
El botón ha sido presionado 1 veces
El botón ha sido presionado 2 veces
El botón ha sido presionado 3 veces
...
Interrupción desactivada
```

### Especificaciones del Código
El código consta de:
- **Estructura `Button`** que almacena:
  - `PIN`: Número del pin asignado.
  - `numberKeyPresses`: Contador de pulsaciones.
  - `pressed`: Estado del botón.
- **Funciones principales:**
  - `isr()`: Incrementa el contador y marca el botón como presionado.
  - `setup()`: Configura el pin como entrada con **pull-up interno** y adjunta la interrupción.
  - `loop()`: Verifica si el botón ha sido presionado e imprime el contador. También desactiva la interrupción tras 1 minuto de inactividad.

## 2.- Práctica B: Interrupción por Temporizador (Timer)

### Código:
```cpp
volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onTimer() {
portENTER_CRITICAL_ISR(&timerMux);
interruptCounter++;
portEXIT_CRITICAL_ISR(&timerMux);
}
void setup() {
Serial.begin(115200);
timer = timerBegin(0, 80, true);
timerAttachInterrupt(timer, &onTimer, true);
timerAlarmWrite(timer, 1000000, true);
timerAlarmEnable(timer);
}
void loop() {
if (interruptCounter > 0) {
portENTER_CRITICAL(&timerMux);
interruptCounter--;
portEXIT_CRITICAL(&timerMux);
totalInterruptCounter++;
Serial.print("An interrupt as occurred. Total number: ");
Serial.println(totalInterruptCounter);
}
}
```

### Funcionamiento y Salidas
En esta parte, se genera una interrupción cada **1 segundo** utilizando un temporizador (timer). Cada vez que se activa la interrupción, el contador global se incrementa y se muestra el número total de interrupciones en el puerto serie.

#### Salida esperada en el puerto serie:
```
Se ha producido una interrupción. Total: 1
Se ha producido una interrupción. Total: 2
Se ha producido una interrupción. Total: 3
...
```

### Especificaciones del Código
Este código incluye:
- **Función de Interrupción del Temporizador (`onTimer`)**: Incrementa el contador de interrupciones dentro de una sección crítica.
- **`setup()`**: Configura el temporizador para generar una interrupción cada 1 segundo y lo habilita.
- **`loop()`**: Comprueba si se ha producido una interrupción, actualiza los contadores y muestra el resultado por el puerto serie.

## Conclusión
Esta práctica muestra cómo se pueden manejar interrupciones en el ESP32 mediante dos métodos diferentes:
1. **Interrupción por GPIO**: Se activa manualmente al pulsar un botón.
2. **Interrupción por Timer**: Se activa automáticamente cada segundo.

Ambos métodos permiten ejecutar código de forma asíncrona, sin depender del `loop()`, lo que es esencial en aplicaciones de tiempo real y sistemas embebidos.

