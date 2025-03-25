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

### Funcionamiento
Se configura un temporizador para generar interrupciones cada 1 segundo (1.000.000 microsegundos).
Cada vez que se activa una interrupción, el valor de interruptCounter se incrementa.
En el bucle principal (loop()), si se detecta que ha ocurrido una interrupción (interruptCounter es mayor que 0), el contador se reduce y se añade al total de interrupciones registradas (totalInterruptCounter).
Finalmente, el número total de interrupciones se imprime en el monitor serie.

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
### **Ejercicio de mejora:**

**Codigo main.cpp:**

```cpp
#include <Arduino.h>
const int LED_PIN = 4;       // LED en GPIO4
const int BTN_UP = 18;       // Botón para aumentar la frecuencia
const int BTN_DOWN = 17;     // Botón para disminuir la frecuencia


volatile int interruptCounter = 0;
volatile int blinkDelay = 500; // Frecuencia inicial (500ms -> 1Hz)
volatile bool ledState = false;
volatile unsigned long lastPressUp = 0;
volatile unsigned long lastPressDown = 0;
const int debounceTime = 200; // Tiempo de debounce en ms


hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}


void IRAM_ATTR checkButtons() {
    unsigned long currentMillis = millis();


    // Comprobamos si el botón UP fue presionado (con debounce)
    if (digitalRead(BTN_UP) == LOW && (currentMillis - lastPressUp > debounceTime)) {
        lastPressUp = currentMillis;
        if (blinkDelay > 100) { // Evitar que sea demasiado rápido
            blinkDelay -= 50; // Aumenta la frecuencia
        }
    }


    // Comprobamos si el botón DOWN fue presionado (con debounce)
    if (digitalRead(BTN_DOWN) == LOW && (currentMillis - lastPressDown > debounceTime)) {
        lastPressDown = currentMillis;
        if (blinkDelay < 2000) { // Evitar que sea demasiado lento
            blinkDelay += 50; // Disminuye la frecuencia
        }
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);


    // Configurar el temporizador
    timer = timerBegin(0, 80, true); // Timer 0, divisor 80 → 1 tick = 1µs
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 500000, true); // 500ms inicial (1Hz)
    timerAlarmEnable(timer);


    Serial.println("Sistema iniciado...");
}


void loop() {
    if (interruptCounter > 0) {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);


        // Alternar estado del LED
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);


        // Verificar pulsadores dentro del timer para evitar rebotes
        checkButtons();


        // Actualizar el tiempo del temporizador según la frecuencia modificada
        timerAlarmWrite(timer, blinkDelay * 1000, true);
    }
}


```
Lo que nos pide en este apartado extra es que con un nuevo código podamos alterar la frecuencia de parpadeo de un led utilizando 2 pulsadores. Por tanto el led parpadeara con la frecuencia inicial e irá teniendo interrupciones por timer, y luego cuando pulses un botón la frecuencia de parpadeo aumentará y si pulsas el contrario disminuye. También el código actúa como filtraje de pulsadores para evitar rebotes usando la función checkButtons(). En nuestro caso el botón que aumenta la frecuencia es el del GPIO18 y el que disminuye la frecuencia es el del GPIO17, y el led está situado en el GPIO4. Por tanto nuestro led empieza a parpadear a 1Hz (500ms) de frecuencia inicial y sale por pantalla su respectivo mensaje de que se ha iniciado el sistema. Por tanto como hemos indicado anteriormente cuando pulsamos el botón del GPIO18 disminuye de 50 en 50 el tiempo entre parpadeos causando que la frecuencia sea mayor con un mínimo de 100ms, y cuando se presione el GPIO17 habrá la misma proporción de disminución que de aumento en el GPIO18 solo que en el caso de la bajada el parpadeo tendrá el limite en 2000ms.



