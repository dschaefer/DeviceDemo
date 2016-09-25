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
#endif

GPIO::GPIO() {
	// TODO Auto-generated constructor stub

}

void GPIO::pinMode(uint32_t pin, PinMode mode) {
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
}

void GPIO::interruptEnable(uint32_t pin, InterruptMode mode) {
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
}

void GPIO::interruptDisable(uint32_t pin) {
	uint32_t pin_reg = GPIO_REG_READ(PERIPHS_IO_MUX_GPIO0_U + pin * 4);
	pin_reg &= ~GPIO_PIN_INT_TYPE_MASK;
    GPIO_REG_WRITE(PERIPHS_IO_MUX_GPIO0_U + pin * 4, pin_reg);
}

void GPIO::digitalWrite(uint32_t pin, Level level) {
	switch (level) {
	case Low:
		GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, BIT(pin));
		break;
	case High:
		GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, BIT(pin));
		break;
	}
}

GPIO::Level GPIO::digitalRead(uint32_t pin) {
	if (GPIO_REG_READ(GPIO_IN_ADDRESS) & BIT(pin)) {
		return High;
	} else {
		return Low;
	}
}
