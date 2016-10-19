#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GPIO.h>
#include <NeoPixel.h>
#include <MQTTClient/src/MQTTClient.h>
#include "DeviceDemo.h"

#define MQTT_SERVER "192.168.0.100"

NeoPixel pixels(4, PIXEL_PIN);
IPStack ipstack;
MQTT::Client<IPStack, Countdown> mqttClient(ipstack);
MQTTPacket_connectData connectParams = MQTTPacket_connectData_initializer;

uint32_t sonar_start;
uint32_t sonar_time;

static void set_color(uint8_t r, uint8_t g, uint8_t b) {
	for (int i = 0; i < pixels.getNumPixels(); i++) {
		pixels.setPixel(i, r, g, b);
	}
	pixels.show();
}

int state = -1;

static void send_color() {
	const char *color;
	switch (state) {
	case 0:
		color = "green";
		break;
	case 1:
		color = "yellow";
		break;
	case 2:
		color = "red";
		break;
	default:
		color = "unknown";
	}
	printf("%s\n", color);
	while (mqttClient.publish("/dist", (char *) color, strlen(color))) {
		delay(100);
	}
}

bool started = false;

static bool range_finder_isr(GPIO * pin) {
	if (!started) {
		sonar_start = system_get_time();
		started = true;
		pin->interruptMode(GPIO::NegEdge);
		return false;
	} else {
		sonar_time = system_get_time() - sonar_start;
		started = false;
		pin->interruptMode(GPIO::PosEdge);
		return true;
	}
}

bool in_flash_mode = false;

static void flash_mode() {
	while (in_flash_mode) {
		set_color(0, 0, 150);
		while (mqttClient.publish("/dist", (void *) "blue", 4)) {
			delay(100);
		}
		delay(200);
		set_color(0, 0, 0);
		while (mqttClient.publish("/dist", (void *) "black", 5)) {
			delay(100);
		}
		delay(200);
	}
	send_color();
}

static void *range_finder_task(void * arg) {
	char message[16];
	GPIO triggerPin(TRIGGER_PIN);
	GPIO echoPin(ECHO_PIN);

	set_color(0, 100, 0);

	triggerPin.pinMode(GPIO::Output);
	triggerPin.digitalWrite(GPIO::Low);

	echoPin.pinMode(GPIO::Input);
	echoPin.interruptMode(GPIO::PosEdge);
	echoPin.interruptEnable(range_finder_isr);

	while (true) {
		if (in_flash_mode) {
			flash_mode();
		}

		triggerPin.digitalWrite(GPIO::High);
		usleep(20);
		triggerPin.digitalWrite(GPIO::Low);

		// suspend until interrupt arrives
		echoPin.interruptWait();

		uint32_t dist = sonar_time / 58;

		int newState;
		if (dist < 20) {
			set_color(100, 0, 0);
			newState = 2;
		} else if (dist < 40) {
			set_color(100, 100, 0);
			newState = 1;
		} else {
			set_color(0, 100, 0);
			newState = 0;
		}

		if (newState != state) {
			state = newState;
			send_color();
		}
		sprintf(message, "%d", dist);

		delay(100);
	}
}

void command_arrived(MQTT::MessageData& md) {
	MQTT::Message &message = md.message;

	char buff[32];
	memcpy(buff, message.payload, message.payloadlen);
	buff[message.payloadlen] = 0;
	printf("Received: %s\n", buff);
	if (!strcmp(buff, "Ping")) {
		send_color();
	} else if (!strcmp(buff, "Flash")) {
		in_flash_mode = true;
	} else if (!strcmp(buff, "Off")) {
		in_flash_mode = false;
	}
}

void reconnect() {
	mqttClient.disconnect();
	ipstack.disconnect();
	delay(100);
	printf("Reconnnecting...\n");
	ipstack.ip_connect(MQTT_SERVER, 1883);
	mqttClient.connect(connectParams);
	mqttClient.subscribe("/command", MQTT::QOS0, command_arrived);
}

void *mqtt_task(void * arg) {
	ipstack.ip_connect(MQTT_SERVER, 1883);
	connectParams.clientID.cstring = (char *) "Device";
	mqttClient.connect(connectParams);
	printf("MQTT Connected\n");

	// start up the range finder
	range_finder_start(range_finder_task);

	mqttClient.subscribe("/command", MQTT::QOS0, command_arrived);

	while (true) {
		if (mqttClient.yield()) {
			reconnect();
		}
	}
}

void demo_start() {
	set_color(0, 0, 150);
	mqtt_start(mqtt_task);
}
