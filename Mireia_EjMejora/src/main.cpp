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

