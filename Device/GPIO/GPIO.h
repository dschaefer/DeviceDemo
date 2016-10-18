/*
 * GPIO.h
 *
 *  Created on: Sep 24, 2016
 *      Author: dschaefer
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

#if defined(ESP8266)
#include <esp_common.h>
#elif defined(__QNX__)
#include <sys/siginfo.h>

#define QNX_PIN(gpio, pin) ((gpio << 5) | (pin & 0x1f))

#define HW_WRITE32(reg, value) *((volatile uint32_t *) (reg)) = value;
#define HW_READ32(reg) (*((volatile uint32_t *)(reg)))
#define HW_BITSET32(reg, bit) HW_WRITE32(reg, HW_READ32(reg) | (1 << (bit)))
#define HW_BITCLR32(reg, bit) HW_WRITE32(reg, HW_READ32(reg) & ~(1 << (bit)))

#define GPIO_REVISION              0x000
#define GPIO_SYSCONFIG             0x010
#define GPIO_EOI                   0x020
#define GPIO_IRQSTATUS_RAW_0       0x024
#define GPIO_IRQSTATUS_RAW_1       0x028
#define GPIO_IRQSTATUS_0           0x02C
#define GPIO_IRQSTATUS_1           0x030
#define GPIO_IRQSTATUS_SET_0       0x034
#define GPIO_IRQSTATUS_SET_1       0x038
#define GPIO_IRQSTATUS_CLR_0       0x03C
#define GPIO_IRQSTATUS_CLR_1       0x040
#define GPIO_SYSSTATUS             0x114
#define GPIO_CTRL                  0x130
#define GPIO_OE                    0x134
#define GPIO_DATAIN                0x138
#define GPIO_DATAOUT               0x13C
#define GPIO_LEVELDETECT0          0x140
#define GPIO_LEVELDETECT1          0x144
#define GPIO_RISINGDETECT          0x148
#define GPIO_FALLINGDETECT         0x14C
#define GPIO_DEBOUNCENABLE         0x150
#define GPIO_DEBOUNCINGTIME        0x154
#define GPIO_CLEARDATAOUT          0x190
#define GPIO_SETDATAOUT            0x194
#endif

class GPIO {
public:
	GPIO(uint8_t pin);
	~GPIO();

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

	uint8_t getPin();

	void pinMode(PinMode mode);

	typedef bool (*handler)(GPIO * pin);

	void interruptEnable(handler interrupt_handler);
	void interruptMode(InterruptMode mode);
	void interruptWait();
	void interruptDisable();

	void digitalWrite(Level level) __attribute__((always_inline)) {
		switch (level) {
		case Low:
#if defined(ESP8266)
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT(pin));
#elif defined(__QNX__)
			HW_WRITE32(reg[gpio] + GPIO_CLEARDATAOUT, 1 << pin);
#endif
			break;
		case High:
#if defined(ESP8266)
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT(pin));
#elif defined(__QNX__)
			HW_WRITE32(reg[gpio] + GPIO_SETDATAOUT, 1 << pin);
#endif
			break;
		}
	}

	Level digitalRead() __attribute__((always_inline)) {
#if defined(ESP8266)
		return (GPIO_REG_READ(GPIO_IN_ADDRESS) & BIT(pin)) ? High : Low;
#elif defined(__QNXNTO__)
		return (HW_READ32(reg[gpio] + GPIO_DATAIN) & (1 << pin)) ? High : Low;
#endif
	}

	handler getInterruptHandler() {
		return interrupt_handler;
	}

#if defined(ESP8266)
	xTaskHandle getTaskHandle() {
		return task_handle;
	}
#elif defined(__QNXNTO__)
	uint32_t getIRQ() {
		return 0x300 + gpio * 32 + pin;
	}

	struct sigevent * getEvent() {
		return &event;
	}
#endif

private:
	handler interrupt_handler;
#if defined(ESP8266)
	uint8_t pin;
	xTaskHandle task_handle;
#elif defined(__QNXNTO__)
	uint8_t gpio;
	uint8_t pin;
	static uint8_t *reg[];
	int iid;
	struct sigevent event;
#endif
};

#endif /* GPIO_H_ */
