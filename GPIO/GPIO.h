/*
 * GPIO.h
 *
 *  Created on: Sep 24, 2016
 *      Author: dschaefer
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

class GPIO {
public:
	GPIO();
	~GPIO() { }

	enum PinMode {
		Output,
		Input,
		InputPullup
	};

	enum Level {
		Low = 0,
		High = 1
	};

	enum InterruptMode {
		PosEdge,
		NegEdge
	};

	void pinMode(uint32_t pin, PinMode mode);

	void interruptEnable(uint32_t pin, InterruptMode mode);
	void interruptDisable(uint32_t pin);

	void digitalWrite(uint32_t pin, Level level);
	Level digitalRead(uint32_t pin);
};

#endif /* GPIO_H_ */
