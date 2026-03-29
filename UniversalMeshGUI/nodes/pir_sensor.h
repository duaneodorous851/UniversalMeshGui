#pragma once
#ifdef HAS_PIR

#include <Arduino.h>

#define PIR_GPIO        36
#define PIR_COOLDOWN_MS 5000    // minimum ms between mesh alerts
#define PIR_BLINK_COUNT 3       // number of LED flashes on trigger
#define PIR_BLINK_MS    100     // ms per on/off phase

typedef void (*PirTriggerCallback)(void);

void pirSetup(PirTriggerCallback cb);
void pirLoop();

#endif // HAS_PIR
