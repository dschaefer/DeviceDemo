#if defined(ESP8266)

#include "DeviceDemo.h"
#include <esp_common.h>

extern "C" void uart_div_modify(int, int);

// Memory to load certs into
char certs[2700] __attribute__((aligned(4)));

xTaskHandle range_finder_handle;

extern "C" void user_init() {
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	printf("\n");

	// load up certs flash page
	spi_flash_read(0x70000, (uint32_t *)certs, sizeof(certs));

	// Set up wifi
	wifi_set_opmode(STATION_MODE);
	struct station_config config;
	memset(&config, 0, sizeof(config));
	sprintf((char *) config.ssid, "dasWifi");
	sprintf((char *) config.password, certs);
	wifi_station_set_config(&config);
	wifi_station_connect();

	demo_start();
}

void mqtt_start(task mqtt_task) {
	xTaskCreate((pdTASK_CODE) mqtt_task, (const signed char *) "mqtt", 256, NULL, 1, NULL);
}

void range_finder_start(task range_finder_task) {
	xTaskCreate((pdTASK_CODE) range_finder_task, (const signed char *) "range_finder", 256, NULL, 2, &range_finder_handle);
}

void range_finder_suspend() {
	vTaskSuspend(range_finder_handle);
}

void range_finder_wake() {
}

#endif
