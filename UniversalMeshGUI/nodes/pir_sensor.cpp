#include <Arduino.h>
#include "pir_sensor.h"

#ifdef HAS_PIR

#if defined(ESP8266)
  #define LED_ON  LOW    // ESP8266 onboard LED is active LOW
  #define LED_OFF HIGH
#else
  #define LED_ON  HIGH   // ESP32 onboard LED is active HIGH
  #define LED_OFF LOW
#endif

#ifndef LED_BUILTIN
  #define LED_BUILTIN 2  // GPIO2 fallback for boards that don't define it
#endif

static PirTriggerCallback _pirCallback  = nullptr;
static unsigned long      _lastTrigger  = 0;
static int                _blinkCount   = 0;   // remaining half-cycles (on+off = 2 each flash)
static unsigned long      _lastBlink    = 0;

void pirSetup(PirTriggerCallback cb) {
    _pirCallback = cb;
    pinMode(PIR_GPIO, INPUT);   // GPIO36 is input-only; no pull-up/down available
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LED_OFF);
    Serial.printf("[PIR] Initialized on GPIO%d, cooldown=%dms\n", PIR_GPIO, PIR_COOLDOWN_MS);
}

void pirLoop() {
    unsigned long now = millis();

    // Non-blocking LED blink
    if (_blinkCount > 0 && now - _lastBlink >= PIR_BLINK_MS) {
        _lastBlink = now;
        bool ledOn = (_blinkCount % 2 == 0);   // even = on, odd = off
        digitalWrite(LED_BUILTIN, ledOn ? LED_ON : LED_OFF);
        _blinkCount--;
        if (_blinkCount == 0) digitalWrite(LED_BUILTIN, LED_OFF);
    }

    if (digitalRead(PIR_GPIO) == HIGH) {
        if (now - _lastTrigger >= PIR_COOLDOWN_MS) {
            _lastTrigger = now;
            _blinkCount  = PIR_BLINK_COUNT * 2;  // each flash = one ON + one OFF half-cycle
            _lastBlink   = now;
            digitalWrite(LED_BUILTIN, LED_ON);    // start first flash immediately
            Serial.println("[PIR] Motion detected!");
            if (_pirCallback) _pirCallback();
        }
    }
}

#endif // HAS_PIR
