#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(4, 13);

void setup() {
	pixels.begin();
}

void loop() {
	for (int i = 0; i < 4; i++) {
		//pixels.setPixelColor(i, 0, 0, 0);
		pixels.setPixelColor(i, rand(), rand(), rand());
	}
	pixels.show();

	delay(200);
}
