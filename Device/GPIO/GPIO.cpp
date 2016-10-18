#include "../GPIO/GPIO.h"

#if defined(ESP8266)
#include <esp_common.h>

static struct {
	volatile uint32_t pin;
	uint32_t func;
} pinMap[16] = {
		{ PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0 },	// 0
		{ PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1 },	// 1
		{ PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2 },	// 2
		{ PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3 },	// 3
		{ PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4 },	// 4
		{ PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5 },	// 5
		{ PERIPHS_IO_MUX_SD_CLK_U, FUNC_GPIO6 },	// 6
		{ PERIPHS_IO_MUX_SD_DATA0_U, FUNC_GPIO7 },	// 7
		{ PERIPHS_IO_MUX_SD_DATA1_U, FUNC_GPIO8 },	// 8
		{ PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9 },	// 9
		{ PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10 },	// 10
		{ PERIPHS_IO_MUX_SD_CMD_U, FUNC_GPIO11 },	// 11
		{ PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12 },	// 12
		{ PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13 },	// 13
		{ PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14 },	// 14
		{ PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15 },	// 15
};
#elif defined(__QNXNTO__)
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/neutrino.h>

uint8_t *GPIO::reg[4] = { NULL, NULL, NULL, NULL };
uint8_t *cm_reg = NULL;

#endif

GPIO::GPIO(uint8_t _pin)
: interrupt_handler(NULL)
#if defined(ESP8266)
, pin(_pin), task_handle(NULL) {
#elif defined(__QNXNTO__)
, gpio(_pin >> 5), pin(_pin & 0x1f), iid(0) {
	SIGEV_NONE_INIT(&event);
	ThreadCtl(_NTO_TCTL_IO_PRIV, NULL);

	// Control Module Registers
	if (cm_reg == NULL) {
		cm_reg = (uint8_t *) mmap_device_memory(NULL, 0x2000, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, 0x44e10000);
		// TODO Don't hard code this so much
		HW_WRITE32(cm_reg + 0x870, 0x27);
		HW_WRITE32(cm_reg + 0x874, 0x27);
		HW_WRITE32(cm_reg + 0x840, 0x27);
	}

	// GPIO Registers
	if (reg[gpio] == NULL) {
		uint32_t address = 0;
		switch (gpio) {
		case 0:
			address = 0x44e07000;
			break;
		case 1:
			address = 0x4804c000;
			break;
		case 2:
			address = 0x481ac000;
			break;
		case 3:
			address = 0x481ae000;
			break;
		}
		reg[gpio] = (uint8_t *) mmap_device_memory(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, address);
	}
#endif
}

GPIO::~GPIO() {
}

uint8_t GPIO::getPin() {
#if defined(ESP8266)
	return pin;
#elif defined(__QNXNTO__)
	return QNX_PIN(gpio, pin);
#endif
}

void GPIO::pinMode(PinMode mode) {
#if defined(ESP8266)
	PIN_FUNC_SELECT(pinMap[pin].pin, pinMap[pin].func);
	switch (mode) {
	case Output:
		GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, BIT(pin));
		break;
	case Input:
		GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, BIT(pin));
		break;
	case InputPullup:
		GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, BIT(pin));
		PIN_PULLUP_EN(pinMap[pin].pin);
		break;
	}
#elif defined(__QNX__)
	switch (mode) {
	case Output:
		HW_BITCLR32(reg[gpio] + GPIO_OE, pin);
		break;
	case Input:
		HW_BITSET32(reg[gpio] + GPIO_OE, pin);
		break;
	}
#endif
}

#if defined(ESP8266)
static void isr_handler(void * arg) {
	GPIO * pin = (GPIO *) arg;
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	if (gpio_status & BIT(pin->getPin())) {
		if (pin->getInterruptHandler()(pin)) {
			xTaskResumeFromISR(pin->getTaskHandle());
		}
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(pin->getPin()));
	}
}
#elif defined(__QNXNTO__)
static const sigevent *isr_handler(void * arg, int id) {
	GPIO * pin = (GPIO *) arg;
	if (pin->getInterruptHandler()(pin)) {
		return pin->getEvent();
	} else {
		return NULL;
	}
}
#endif

void GPIO::interruptEnable(handler _interrupt_handler) {
    interrupt_handler = _interrupt_handler;

#if defined(ESP8266)
	portENTER_CRITICAL();

	// TODO what if we have multiple handlers?
	_xt_isr_attach(ETS_GPIO_INUM, isr_handler, this);
    _xt_isr_unmask(1 << ETS_GPIO_INUM);

	portEXIT_CRITICAL();
#elif defined(__QNXNTO__)
	SIGEV_INTR_INIT(&event);
	iid = InterruptAttach(getIRQ(), isr_handler, this, 0, 0);
#endif
}

void GPIO::interruptMode(InterruptMode mode) {
#if defined(ESP8266)
	uint32_t state;
	switch (mode) {
	case PosEdge:
		state = 1;
		break;
	case NegEdge:
		state = 2;
		break;
	}

	uint32_t pin_reg = GPIO_REG_READ(GPIO_PIN0_ADDRESS + pin * 4);
    pin_reg &= ~GPIO_PIN_INT_TYPE_MASK;
    pin_reg |= state << GPIO_PIN_INT_TYPE_LSB;
    GPIO_REG_WRITE(GPIO_PIN0_ADDRESS + pin * 4, pin_reg);
#elif defined(__QNXNTO__)
    switch (mode) {
    case PosEdge:
    	HW_BITCLR32(reg[gpio] + GPIO_LEVELDETECT0, pin);
    	HW_BITCLR32(reg[gpio] + GPIO_LEVELDETECT1, pin);
    	HW_BITSET32(reg[gpio] + GPIO_RISINGDETECT, pin);
    	HW_BITCLR32(reg[gpio] + GPIO_FALLINGDETECT, pin);
    	break;
    case NegEdge:
    	HW_BITCLR32(reg[gpio] + GPIO_LEVELDETECT0, pin);
    	HW_BITCLR32(reg[gpio] + GPIO_LEVELDETECT1, pin);
    	HW_BITCLR32(reg[gpio] + GPIO_RISINGDETECT, pin);
    	HW_BITSET32(reg[gpio] + GPIO_FALLINGDETECT, pin);
    	break;
    }
#endif
}

void GPIO::interruptWait() {
#if defined(ESP8266)
	task_handle = xTaskGetCurrentTaskHandle();
	vTaskSuspend(task_handle);
#elif defined(__QNXNTO__)
	InterruptWait(0, NULL);
#endif
}

void GPIO::interruptDisable() {
#if defined(ESP8266)
	uint32_t pin_reg = GPIO_REG_READ(PERIPHS_IO_MUX_GPIO0_U + pin * 4);
	pin_reg &= ~GPIO_PIN_INT_TYPE_MASK;
    GPIO_REG_WRITE(PERIPHS_IO_MUX_GPIO0_U + pin * 4, pin_reg);
#elif defined(__QNXNTO__)
    InterruptDetach(iid);
#endif
}
