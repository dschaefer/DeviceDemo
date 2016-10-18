#ifndef __DEVICEDEMO_H_
#define __DEVICEDEMO_H_

#if defined(ESP8266)
#include "DeviceESP8266.h"
#else
#include "DeviceQNX.h"
#endif

extern void demo_start();

typedef void *(*task)(void * arg);

extern void mqtt_start(task mqtt_task);

extern void range_finder_start(task range_finder_task);
extern void range_finder_suspend();
extern void range_finder_wake();

#endif // __DEVICEDEMO_H_
