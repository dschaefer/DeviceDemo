#ifndef __DEVICEQNX_H_
#define __DEVICEQNX_H_

#include <MQTTClient/src/qnx/qnx.h>

#define PIXEL_PIN	QNX_PIN(1, 16)
#define TRIGGER_PIN	QNX_PIN(0, 30)
#define ECHO_PIN	QNX_PIN(0, 31)

extern uint32_t system_get_time();

#endif // __DEVICEQNX_H_
