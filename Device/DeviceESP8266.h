#ifndef __DEVICEESP8266_H_
#define __DEVICEESP8266_H_

#include <MQTTClient/src/esp8266/ESP8266.h>

#define PIXEL_PIN	13
#define TRIGGER_PIN	14
#define ECHO_PIN		12

#define usleep(us)	os_delay_us(us)
#define delay(ms)	vTaskDelay(ms / portTICK_RATE_MS)

#endif // __DEVICEESP8266_H_
