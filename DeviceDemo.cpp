#include <NeoPixel.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(ESP8266)
#include <esp_common.h>
#include <GPIO.h>

#define PIXEL_PIN	14
#define TRIGGER_PIN	13
#define ECHO_PIN	12

extern "C" void uart_div_modify(int, int);

#elif defined(__QNX__)

// gpio1[16]
#define PIXEL_PIN	16

#endif

NeoPixel pixels(4, PIXEL_PIN);
xTaskHandle sonar_handle;

uint32_t start;
uint32_t time;

static void set_color(uint8_t r, uint8_t g, uint8_t b) {
	for (int i = 0; i < pixels.getNumPixels(); i++) {
		pixels.setPixel(i, r, g, b);
	}
	pixels.show();
}

bool started = false;

static void sonar_isr(void * arg) {
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	if (gpio_status & BIT(ECHO_PIN)) {
		GPIO * gpio = (GPIO *) arg;
		if (!started) {
			start = system_get_time();
			started = true;
			gpio->interruptEnable(ECHO_PIN, GPIO::NegEdge);
		} else {
			time = system_get_time() - start;
			started = false;
			gpio->interruptEnable(ECHO_PIN, GPIO::PosEdge);
			xTaskResumeFromISR(sonar_handle);
		}
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(ECHO_PIN));
	}
}

static void sonar_task(void * arg) {
	GPIO gpio;

	gpio.pinMode(TRIGGER_PIN, GPIO::Output);
	gpio.pinMode(ECHO_PIN, GPIO::Input);

	set_color(0, 150, 0);

	portENTER_CRITICAL();

	gpio.interruptEnable(ECHO_PIN, GPIO::PosEdge);
	_xt_isr_attach(ETS_GPIO_INUM, sonar_isr, &gpio);
    _xt_isr_unmask(1 << ETS_GPIO_INUM);

	portEXIT_CRITICAL();

	printf("Startup\n");

	while (true) {
		gpio.digitalWrite(TRIGGER_PIN, GPIO::High);
		os_delay_us(20);
		gpio.digitalWrite(TRIGGER_PIN, GPIO::Low);

		vTaskSuspend(NULL);

		uint32_t dist = time / 58;

		if (dist < 20) {
			set_color(150, 0, 0);
		} else if (dist < 40) {
			set_color(150, 150, 0);
		} else {
			set_color(0, 150, 0);
		}

		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

#if defined(ESP8266)

extern "C" void user_init() {
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	printf("\n");

	xTaskCreate(sonar_task, (const signed char *) "sonar", 256, NULL, 2, &sonar_handle);
}

#elif defined(__QNX__)

int main(int argc, char **argv) {
	return 0;
}

#endif
